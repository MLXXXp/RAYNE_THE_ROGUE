#include <EEPROM.h>
#include <Arduboy2.h>
#include "sfs.h"

// Constructor
// filename is the name of the new file

sfs::sfs() {
}

sfs::sfs(const char *  fname, uint8_t fsize) {
	strncpy(fileName,fname,6);
  fileSize = fsize;
	GetFileAddress();
}


void sfs::GetFileAddress() {
  startAddress = 0;
  for (uint16_t i = EEPROM_STORAGE_SPACE_START; i < EEPROM.length()-7-fileSize; i++) {
    //find f start
    if (EEPROM.read(i) == fileName[0] &&
      EEPROM.read(i+1) == fileName[1] &&
      EEPROM.read(i+2) == fileName[2] &&
      EEPROM.read(i+3) == fileName[3] &&
      EEPROM.read(i+4) == fileName[4] &&
      EEPROM.read(i+5) == fileName[5]){
      startAddress = i;
      fileSize = EEPROM.read(i+6);
      break;
    }
  }
}

bool sfs::Exists(){
  if(startAddress>0){
    return true;
  }
  return false;
}

bool sfs::Load() {
  GetFileAddress();
  if(startAddress!=0){
    for (unsigned int t=0; t<sizeof(game_data); t++){
      *((char*)&game_data + t) = EEPROM.read(startAddress + 7 + t);
    }
    return true;
  }
  return false;
}

uint16_t sfs::newfileStartIndex(uint8_t fSize){
  uint16_t empty_bytes=0;
  uint16_t new_start_index = 0;
  uint16_t i = EEPROM_STORAGE_SPACE_START;
  do{
    if(EEPROM.read(i)==0xFF && new_start_index==0){//found an empty byte
      new_start_index=i;
      empty_bytes++;
    }else if(EEPROM.read(i)==0xFF && new_start_index!=0){//found an empty byte
      empty_bytes++;
      if(empty_bytes==fSize){
        return new_start_index;
      }
    }else{
      new_start_index=0;
      empty_bytes=0;
    }
    i++;
  }while(i < EEPROM.length()-fSize);
  return 0;
}

bool sfs::Save() {
  GetFileAddress();
  if(startAddress==0){//new f
    startAddress = newfileStartIndex(sizeof(game_data)+7);// find free space to save the f
    if(startAddress!=0){//found space
      //write fname
      for(uint16_t i=0;i<6;i++){// write the f
        EEPROM.update(startAddress+i, *((char*)&fileName + i) );
      }
      //write fsize
      EEPROM.update(startAddress+6, fileSize);
      //write fdata
      for(uint16_t i=0;i<sizeof(game_data);i++){// write the f
        EEPROM.update(startAddress+7+i, *((char*)&game_data + i) );
      }   
      return true;
    }else{
      return false;
    }
  }else{//update existing f
    if(sizeof(game_data) == fileSize){// is new data the same size      
      //write fname
      for(uint16_t i=0;i<6;i++){// write the f
        EEPROM.update(startAddress+i, *((char*)&fileName + i) );
      }
      //write fsize
      EEPROM.update(startAddress+6, fileSize);
      //write fdata
      for(uint16_t i=0;i<sizeof(game_data);i++){// write the f
        EEPROM.update(startAddress+7+i, *((char*)&game_data + i) );
      } 
    }else{
      uint16_t tstartIndex =newfileStartIndex(sizeof(game_data)+7);// find free space to save the f      
      if(tstartIndex!=0){// found enough space
        // delete old f, set all bytes to 0        
        for(uint16_t i=0;i<sizeof(game_data)+7;i++){
          EEPROM.update(startAddress+i, 0);
        } 
        //write fname
        for(uint16_t i=0;i<6;i++){// write the f
          EEPROM.update(tstartIndex+i, *((char*)&fileName + i) );
        }
        //write fsize
        EEPROM.update(tstartIndex+6, fileSize);
        //write fdata
        for(uint16_t i=0;i<sizeof(game_data);i++){// write the f
          EEPROM.update(tstartIndex+7+i, *((char*)&game_data + i) );
        } 
        startAddress=tstartIndex;
        return true;
      }else{
        return false;
      }    
    }
  }
}

bool sfs::Erase(){
  if(startAddress!=0){//if no start index then f not found
    for (uint16_t i=0; i <=7+fileSize; i++) {
      EEPROM.update(startAddress+i, 0xFF);
    }
    startAddress=0;
    return true;
  }
  return false;
}

void sfs::Clear(){
  for (uint16_t i = EEPROM_STORAGE_SPACE_START ; i < EEPROM.length() ; i++) {
    EEPROM.update(i, 0xFF);
  }
}

sfs::~sfs(){
  delete this;
}

