#define RG_GAMEPAD_ADC1_MAP {\
    {RG_KEY_UP,    ADC1_CHANNEL_6, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_DOWN,  ADC1_CHANNEL_6, ADC_ATTEN_DB_11, 1024, 3072},\
    {RG_KEY_LEFT,  ADC1_CHANNEL_5, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_RIGHT, ADC1_CHANNEL_5, ADC_ATTEN_DB_11, 1024, 3072},\
}
#define RG_GAMEPAD_GPIO_MAP {\
    {RG_KEY_SELECT, GPIO_NUM_16, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_START,  GPIO_NUM_17, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_MENU,   GPIO_NUM_18, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_OPTION, GPIO_NUM_0,  GPIO_FLOATING,    0},\
    {RG_KEY_A,      GPIO_NUM_15, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_B,      GPIO_NUM_5,  GPIO_PULLUP_ONLY, 0},\
}
// Boot key
#define RG_KEY_BOOT                 GPIO_NUM_0

// Battery
#define RG_BATTERY_DRIVER           1
#define RG_BATTERY_ADC_CHANNEL      ADC1_CHANNEL_3
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)

// Status LED
#define RG_GPIO_LED                 GPIO_NUM_38

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_12
#define RG_GPIO_LCD_CLK             GPIO_NUM_48
#define RG_GPIO_LCD_CS              GPIO_NUM_NC
#define RG_GPIO_LCD_DC              GPIO_NUM_47
#define RG_GPIO_LCD_BCKL            GPIO_NUM_39
#define RG_GPIO_LCD_RST             GPIO_NUM_3

// SPI
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_9
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_11
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_13
#define RG_GPIO_SDSPI_CS            GPIO_NUM_10

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_41
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_42
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_40

// I2C
#define RG_I2C_SDA                  GPIO_NUM_2
#define RG_I2C_CLK                  GPIO_NUM_1

// Serial IO
#define RG_TXD                      GPIO_NUM_43
#define RG_RXD                      GPIO_NUM_44
#define RG_USB_DP                   GPIO_NUM_20
#define RG_USB_DM                   GPIO_NUM_19


