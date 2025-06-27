#ifndef __W25Q32_H
#define __W25Q32_H

void W25Q32_Init(void);
void W25Q32_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q32_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);
void W25Q32_SectorErase(uint32_t Address);
void W25Q32_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

#endif
