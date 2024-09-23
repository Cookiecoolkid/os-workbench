#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "fat32.h"


#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

#define MAX_CLUSTERS 65536
#define DIR_ENTRY_SIZE 32
#define BMP_EXT_OFFSET 8

enum { DIRECTORY = 1, BMPHDR, BMPDATA, UNUSED };

typedef struct {
    char name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t createTimeTenth;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t accessDate;
    uint16_t firstClusterHigh;
    uint16_t writeTime;
    uint16_t writeDate;
    uint16_t firstClusterLow;
    uint32_t fileSize;
} __attribute__((packed)) DirEntry;

typedef struct {
    uint8_t seqNumber;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t firstClusterLow;
    uint16_t name3[2];
} __attribute__((packed)) LFNEntry;

#define ATTR_LONG_NAME (0x0F)

void readDirEntry(char* data, int clusterSize) {
    DirEntry* de = (DirEntry*)data;
    LFNEntry* lfn;
    wchar_t fileName[256];
    char tempFileName[256];
		int pos = 0;

    for (int i = 0; i < clusterSize / sizeof(DirEntry); ++i, ++de) {
        if (de->name[0] == 0x00) break;
        if (de->name[0] == 0xe5) continue;

        if (de->attr == ATTR_LONG_NAME) {
            lfn = (LFNEntry*)de;
						
            memcpy(&fileName[pos], lfn->name1, 5 * sizeof(uint16_t));
            pos += 5;
            memcpy(&fileName[pos], lfn->name2, 6 * sizeof(uint16_t));
            pos += 6;
            memcpy(&fileName[pos], lfn->name3, 2 * sizeof(uint16_t));
            pos += 2;


						if ((lfn->seqNumber & 0x40) == 0x40) {
								fileName[pos] = L'\0';
								pos = 0;
                wcstombs(tempFileName, fileName, 256);
								printf("File: %s\n", tempFileName);

								memset(fileName, 0, sizeof(fileName));
						}
        } else if (!(de->attr & ATTR_VOLUME_ID)) {
            memcpy(fileName, de->name, 11 * sizeof(uint16_t));
            fileName[11] = L'\0';
						wcstombs(tempFileName, fileName, 256);
						printf("File: %s\n", tempFileName);
	
						memset(fileName, 0, sizeof(fileName));
        }
    }
}

struct fat32hdr *hdr;
int clusterCategory[MAX_CLUSTERS];
int cluster_size = 0;
int cluster_num = 0;

void *mmap_fat32img(const char* fsimg) {
		int fd = open(fsimg, O_RDONLY);
		if (fd == -1) {
				perror("Error open fsimg");
				return NULL;
		}
		
		off_t size = lseek(fd, 0, SEEK_END);

		struct fat32hdr *hdr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (hdr == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return NULL;
    }
		
		close(fd);

		assert(hdr->Signature_word == 0xaa55); // this is an MBR
		assert(hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec == size);

		/*	
		printf("Tot Clus = %d\n", hdr->BPB_TotSec32 / hdr->BPB_SecPerClus);	
		printf("%s: DOS/MBR boot sector, ", fsimg);
		printf("OEM-ID \"%s\", ", hdr->BS_OEMName);
		printf("sectors/cluster %d, ", hdr->BPB_SecPerClus);
		printf("sectors %d, ", hdr->BPB_TotSec32);
		printf("sectors %d, ", hdr->BPB_TotSec32);
		printf("sectors/FAT %d, ", hdr->BPB_FATSz32);
		printf("serial number 0x%x\n", hdr->BS_VolID);
		*/
		return (void*)hdr;
}


u32 next_cluster(u32 n) {
		u32 FATOffset = 4 * n; // FAT32
		u32 FATSecNum = hdr->BPB_RsvdSecCnt + (FATOffset / hdr->BPB_BytsPerSec);
		u32 FATEntOffset = FATOffset % hdr->BPB_BytsPerSec;

		u8 *SecBuff = (u8 *)hdr + FATSecNum * hdr->BPB_BytsPerSec;
		// DWARD - u32
		u32 FATClusEntryVal = (*((u32 *) &SecBuff[FATEntOffset])) & 0x0FFFFFFF;

		return FATClusEntryVal;
}

void *clus2sec(u32 n) {
		u32 RootDirSectors = ((hdr->BPB_RootEntCnt * 32) + (hdr->BPB_BytsPerSec - 1)) / hdr->BPB_BytsPerSec;
		u32 FirstDataSec = hdr->BPB_RsvdSecCnt + (hdr->BPB_NumFATs * hdr->BPB_FATSz32) + RootDirSectors;

		// Clus#1 Clus#2 not for data
		u32 FirstDataSecOfClus = FirstDataSec + (n - 2) * hdr->BPB_SecPerClus;
		return (void *)((char *)hdr + (FirstDataSecOfClus * hdr->BPB_BytsPerSec));
}

bool is_all_zeros(const char* buffer, size_t size) {
    for (size_t i = 0; i < size; ++i) if (buffer[i] != 0) return false;
    
		return true;
}

int count_cluster_bmp(u32 clusId) {
    char *data = (char*)clus2sec(clusId);
    int count = 0;

    for (int i = 0; i < cluster_size - 2; i++) 
        if (data[i] == 'B' && data[i + 1] == 'M' && data[i + 2] == 'P') count++;        
 
    return count; 
}

int check_cluster_type(u32 clusId) {
    char *data = (char*)clus2sec(clusId);
    if (is_all_zeros(data, cluster_size)) {
				// printf("Find Unused\n");
				return UNUSED;
		}
    if (data[0] == 0x42 && data[1] == 0x4D) {
				// printf("Find BmpHeader\n");
				return BMPHDR;
		}
		int bmpFileCount = count_cluster_bmp(clusId);

    if (bmpFileCount > 1) {
				// printf("Find Directory\n");
        return DIRECTORY;
    }
		// printf("Find BmpData\n");
    return BMPDATA;
}

void TraverseClus() {
		for (int clusId = 2; clusId < cluster_num; clusId += 1) {
				assert(clusId <= MAX_CLUSTERS);
				clusterCategory[clusId] = check_cluster_type(clusId);
		}
}

void TraverseDirectory() {                                                             
		for (int clusId = 2; clusId < cluster_num; clusId += 1) {
				if (clusterCategory[clusId] == DIRECTORY) {
						char* data;
						while (clusId < CLUS_INVALID) {
								data = (char*)clus2sec(clusId);
								readDirEntry(data, cluster_size);
								clusId = next_cluster(clusId); 
						}
				}
		}
}


int main(int argc, char *argv[]) {
		if (argc < 2) {
				fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
				exit(1);
		}

		hdr = (struct fat32hdr*)mmap_fat32img(argv[1]);

		cluster_size = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus;
  	cluster_num = (hdr->BPB_TotSec32 - hdr->BPB_RsvdSecCnt - (hdr->BPB_NumFATs * hdr->BPB_FATSz32)) / hdr->BPB_SecPerClus; 
		TraverseClus();
		TraverseDirectory();
				
		return 0;
}

