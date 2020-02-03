#pragma once


uint16_t ResourcesV;

uint16_t SoftwareV;
long int Soft_slot1_addr;
long int Soft_slot1_len1;

long int Soft_slot2_addr;
long int Soft_slot2_len2;

uint16_t LocalizationV;
long int Local_slot1_addr;
long int Local_slot1_len1;

long int Local_slot2_addr;
long int Local_slot2_len2;

void http_get_task(void);