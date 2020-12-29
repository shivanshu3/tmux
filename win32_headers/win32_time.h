#pragma once

char *ctime_r(const time_t *timep, char *buf);
struct tm *localtime_r(const time_t *timep, struct tm *result);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
