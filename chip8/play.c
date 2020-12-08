/**
 * @file play.c
 *
 * @author Elias
 */
#include "chip8.h"
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define PIXEL_SIZE 4
uint8_t cursorDir;
#define CLOCK_HZ 60
#define CLOCK_RATE_MS ((int)((1.0 / CLOCK_HZ) * 1000 + 0.5))

#define BLACK 0
#define WHITE 255

#define SCREEN_ROWS (GFX_ROWS * PIXEL_SIZE)
#define SCREEN_COLS (GFX_COLS * PIXEL_SIZE)
unsigned char screen[SCREEN_ROWS][SCREEN_COLS][3];

int joyXState;
int joyYState;

extern uint8_t key[KEY_SIZE];
extern uint8_t gfx[GFX_ROWS][GFX_COLS];
extern bool chip8_draw_flag;
Graphics_Context g_sContext;
Graphics_Context g_sContextf;

int gfx2[GFX_ROWS][GFX_ROWS];

void delay_init(void)
{
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_PERIODIC_MODE);

    Timer32_disableInterrupt(TIMER32_0_BASE);
}

void delay(uint32_t duration_us)
{
    Timer32_haltTimer (TIMER32_0_BASE);
    Timer32_setCount  (TIMER32_0_BASE, 48 * duration_us);
    Timer32_startTimer(TIMER32_0_BASE, true);

    while (Timer32_getValue(TIMER32_0_BASE) > 0);
}

void gfx_setup()
{
    // Initialize the LCD
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_LEFT);

    // Initialize context
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);

    // Set colors and fonts
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setFont(&g_sContext, &g_sFontCm12b);

    // Clear the screen
    Graphics_clearDisplay(&g_sContext);
    memset(gfx2,    0, sizeof(uint8_t)  * GFX_SIZE);
    }

int keymap(unsigned char k)
{
    switch (k)
    {
    case '1':
        return 0x1;
    case '2':
        return 0x2;
    case '3':
        return 0x3;
    case '4':
        return 0xc;

    case 'q':
        return 0x4;
    case 'w':
        return 0x5;
    case 'e':
        return 0x6;
    case 'r':
        return 0xd;

    case 'a':
        return 0x7;
    case 's':
        return 0x8;
    case 'd':
        return 0x9;
    case 'f':
        Graphics_clearDisplay(&g_sContext);
        return 0xe;

    case 'z':
        return 0xa;
    case 'x':
        return 0x0;
    case 'c':
        return 0xb;
    case 'v':
        return 0xf;

    default:
        return -1;
    }
}
//remaps input

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
unsigned char inp;
unsigned char getDir()
{

    int yAbs = map(joyYState, 0, 16384, 128, 0);

    int xAbs = map(joyXState, 0, 16384, 0, 128);

    if (~P5IN & 0x02)
    {
        return '1';
    }
    if (~P4IN & 0x02)
    {
        return 'w';
    }
    if (~P3IN & 0x20)
    {
        return 'f';
    }


        if (yAbs > 80)
        {
            return 'w';
        }
        else if(yAbs < 50)
        {
            return '2';
        }


        if (xAbs > 80)
        {
            return 'e';
        }
        else if(xAbs < 50)
        {
            return 'q';
        }

    if (xAbs && yAbs == 64)
    {
        return inp;
    }
    return NULL;
}

void keypress(unsigned char k)
{

    int index = keymap(k);
    if (index >= 0)
    {
        key[index] = 1;
    }
}

void keyrelease(unsigned char k)
{

    int index = keymap(k);
    if (index >= 0)
    {
        key[index] = 0;
    }
}

void paint_cell(int row, int col, int32_t color) {

    int16_t x_2 = 2 * row ;
    int16_t y_2 = 2 * col ;

    int16_t x1 = x_2 ;
    int16_t y1 = y_2 ;
    int16_t x2 = x1 + 1;
    int16_t y2 = y1 + 1;

    Graphics_Rectangle rect = {x1 , y1, x2, y2};

    Graphics_setForegroundColor(&g_sContext, color);
    Graphics_drawRectangle(&g_sContext,&rect);
    Graphics_fillRectangle(&g_sContext, &rect);
}

void draw() {
    int row, col;
    int colf; //collumn flipped
    // Clear framebuffer
    //glClear(GL_COLOR_BUFFER_BIT);
    //Graphics_ClearScreen
    // Draw pixels to the buffer
    //Graphics_clearDisplay(&g_sContext);


    //delay(50);
    for (row = 0; row < GFX_ROWS; row++) {
        for (col = 0; col < GFX_COLS; col++) {
            colf = map(col, 0, 64 , 64, 0);
            if(gfx[row][col] && !gfx2[row][col]){
                paint_cell(row, colf, gfx[row][col] ? GRAPHICS_COLOR_WHITE : GRAPHICS_COLOR_BLACK);
            }
            if(!gfx[row][col] && gfx2[row][col]){
                paint_cell(row, colf, gfx[row][col] ? GRAPHICS_COLOR_WHITE : GRAPHICS_COLOR_BLACK);
                            }
            if(gfx[row][col]){
                gfx2[row][col] = 1;
            }
            else{
                gfx2[row][col] = 0;
                }
            }
        }
    Graphics_flushBuffer(&g_sContext);
    //g_sContext = g_sContextf;
    //Crystalfontz128x128_Init();
    //Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_LEFT);
    // Initialize context
    //Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,&g_sCrystalfontz128x128_funcs);
    //Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    //Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

    // Update Texture
    //glDrawPixels(SCREEN_COLS, SCREEN_ROWS, GL_RGB, GL_UNSIGNED_BYTE,
                 //(void *) screen);
    //glutSwapBuffers();
}

void loop() {

    //Timer32_haltTimer (TIMER32_0_BASE);
    //Timer32_setCount  (TIMER32_0_BASE, 48 * 17166.6666667);
    //Timer32_startTimer(TIMER32_0_BASE, true);

    chip8_emulatecycle();

    if (chip8_draw_flag) {
        draw();
        chip8_draw_flag = false;
    }
    //delay(17.666);
    //chip8_tick();

    if(Timer32_getValue(TIMER32_0_BASE) <= 0){
        chip8_tick();
        Timer32_haltTimer (TIMER32_0_BASE);
        Timer32_setCount  (TIMER32_0_BASE, 48 * 17166.6666667);
        Timer32_startTimer(TIMER32_0_BASE, true);

    }
    //delay(50);
    //chip8_tick();


}

void Init_Graph(Graphics_Context *g_sContext_f)
{
    // Initialize the LCD
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_LEFT);

    // Initialize context
    Graphics_initContext(g_sContext_f, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);

    // Set colors and fonts
    Graphics_setForegroundColor(g_sContext_f, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(g_sContext_f, GRAPHICS_COLOR_BLACK);
    Graphics_setFont(g_sContext_f, &g_sFontCm12b);

    // Clear the screen
    Graphics_clearDisplay(g_sContext_f);
    memset(gfx2,    0, sizeof(uint8_t)  * GFX_SIZE);
}
/*
int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: ./play <game>\n");
        exit(2);
    }

    // Setup Chip8
    chip8_initialize();
    chip8_loadgame(argv[1]);

    // Setup OpenGL
    glutInit(&argc, argv);          
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
 
    glutInitWindowSize(SCREEN_COLS, SCREEN_ROWS);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("chip8");
 
    glutDisplayFunc(draw);
    glutIdleFunc(loop);
    glutReshapeFunc(reshape_window);

    glutKeyboardFunc(keypress);
    glutKeyboardUpFunc(keyrelease);

    gfx_setup();

    Timer32_setCount(TIMER32_0_BASE,20*3*1000000);//3 cycle per microsec
    Timer32_startTimer(TIMER32_0_BASE, true);
    clock_prev.tv_msec = (Timer32_getValue(TIMER32_0_BASE)/3000)%1000;
    clock_prev.tv_sec = Timer32_getValue(TIMER32_0_BASE)/3000000;
 
    // Run the emulator
    glutMainLoop();  

    return 0;
}
*/
void initDelay(void) {
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_PERIODIC_MODE);
    Timer32_disableInterrupt(TIMER32_0_BASE);
}

int main(void)
{
    // Setup Chip8
    chip8_initialize();
    chip8_loadgame("C:\\Users\\smoot\\workspace_v10\\chip8\\SpaceInvaders.ch8");

    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_LEFT);

    /* Initializes graphics context */
    Init_Graph(&g_sContext);

    /* Configures Pin 6.0 and 4.4 as ADC input */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initializing ADC (ADCOSC/64/8) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9)  with repeat)
         * with internal 2.5v reference */
    MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

    MAP_ADC14_configureConversionMemory(ADC_MEM1,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    /* Enabling the interrupt when a conversion on channel 1 (end of sequence)
     *  is complete and enabling conversions */
    MAP_ADC14_enableInterrupt(ADC_INT1);

    /* Enabling Interrupts */
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();

    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();
    delay_init();

    P5->SEL0 &= ~0x02;
    P5->SEL1 &= ~0x02;
    P5->DIR &= ~0x02;
    P5->REN |= 0x02;
    P5OUT |= 0x02;
    P4->SEL0 &= ~0x02;
    P4->SEL1 &= ~0x02;
    P4->DIR &= ~0x02;
    P4->REN |= 0x02;
    P4OUT |= 0x02;


    P3->SEL0 &= ~0x20;
    P3->SEL1 &= ~0x20;
    P3->DIR &= ~0x20;
    P3->REN |= 0x20;
    P3OUT |= 0x20;


    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setFont(&g_sContext, &g_sFontCm12b);

        // Clear the screen
    Graphics_clearDisplay(&g_sContext);

    gfx_setup();
    Graphics_clearDisplay(&g_sContext);
    delay(5*1000);
    //Graphics_clearDisplay(&g_sContext);
    while(1)
    {
        MAP_PCM_gotoLPM0();
    }
}
void ADC14_IRQHandler(void) {

    uint64_t status;

    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    joyXState = ADC14_getResult(ADC_MEM0);
    joyYState = ADC14_getResult(ADC_MEM1);

    if(status & ADC_INT1)
    {
        char k = getDir();
        keypress(k);
        loop();
        keyrelease(k);

    }
}
