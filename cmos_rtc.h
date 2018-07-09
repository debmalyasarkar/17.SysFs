/* CMOS RTC is DS 12887 */

#define CMOS_RTC_NAME   "rtcDevice"

/* Address of RTC Registers */
#define ADDRESS_REG     0x70
#define DATA_REG        0x71

/* Values/Commands for RTC Registers */
#define SECOND          0x00    //Second      00-59
#define MINUTE          0x02    //Minute      00-59
#define HOUR            0x04    //Hour        00-23
#define DAY_OF_WEEK     0x06    //Day of Week 01-0DAY
#define DAY             0x07    //Day         00-31
#define MONTH           0x08    //Month       00-12
#define YEAR            0x09    //Year        00-99

typedef struct _rtc_t
{
  uint32_t second;
  uint32_t minute;
  uint32_t hour;
  uint32_t day;
  uint32_t month;
  uint32_t year;
}rtc_t;
