//
// GUIslice Library Examples
// - Calvin Hass
// - http://www.impulseadventure.com/elec/guislice-gui.html
// - Example 10 (Arduino):
//     Demonstrate textbox controls
//
// ARDUINO NOTES:
// - GUIslice_config.h must be edited to match the pinout connections
//   between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
//

#include "GUIslice.h"
#include "GUIslice_ex.h"
#include "GUIslice_drv.h"


// Defines for resources

// Enumerations for pages, elements, fonts, images
enum {E_PG_MAIN};
enum {E_ELEM_BOX,E_ELEM_BTN_QUIT,E_ELEM_COLOR,
      E_SLIDER,E_ELEM_TXT_COUNT,
      E_ELEM_TEXTBOX,E_SCROLLBAR};
enum {E_FONT_TXT,E_FONT_TITLE};

bool      m_bQuit = false;

// Free-running counter for display
unsigned  m_nCount = 0;

// Instantiate the GUI
#define MAX_PAGE                1
#define MAX_FONT                2

// Define the maximum number of elements per page
// - To enable the same code to run on devices that support storing
//   data into Flash (PROGMEM) and those that don't, we can make the
//   number of elements in Flash dependent upon GSLC_USE_PROGMEM
// - This should allow both Arduino and ARM Cortex to use the same code
#define MAX_ELEM_PG_MAIN          7                                        // # Elems total
#if (GSLC_USE_PROGMEM)
  #define MAX_ELEM_PG_MAIN_PROG   2                                        // # Elems in Flash
#else
  #define MAX_ELEM_PG_MAIN_PROG   0                                         // # Elems in Flash
#endif
#define MAX_ELEM_PG_MAIN_RAM      MAX_ELEM_PG_MAIN - MAX_ELEM_PG_MAIN_PROG  // # Elems in RAM

gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsPage                 m_asPage[MAX_PAGE];
gslc_tsElem                 m_asPageElem[MAX_ELEM_PG_MAIN_RAM];   // Storage for all elements in RAM
gslc_tsElemRef              m_asPageElemRef[MAX_ELEM_PG_MAIN];    // References for all elements in GUI

gslc_tsXSlider              m_sXSlider;
gslc_tsXSlider              m_sXSliderText;

#define TBOX_ROWS           15
#define TBOX_COLS           12
gslc_tsXTextbox             m_sTextbox;
char                        m_acTextboxBuf[TBOX_ROWS*TBOX_COLS];

#define MAX_STR             15

  // Save some element references for quick access
  gslc_tsElemRef*  m_pElemTextbox        = NULL;

// Define debug message function
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

// Quit button callback
bool CbBtnQuit(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
  if (eTouch == GSLC_TOUCH_UP_IN) {
    m_bQuit = true;
  }
  return true;
}

bool CbControls(void* pvGui,void* pvElemRef,int16_t nPos)
{
  gslc_tsGui*     pGui      = (gslc_tsGui*)(pvGui);
  gslc_tsElemRef* pElemRef  = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem*    pElem     = gslc_GetElemFromRef(pGui,pElemRef);

  char            acTxt[20];
  int16_t         nVal;
  gslc_tsElemRef* pElemTmp = NULL;

  // Handle various controls
  switch (pElem->nId) {
    case E_SCROLLBAR:
      // Fetch the scrollbar value
      nVal = gslc_ElemXSliderGetPos(pGui,pElemRef);
      // Update the textbox scroll position
      pElemTmp = gslc_PageFindElemById(pGui,E_PG_MAIN,E_ELEM_TEXTBOX);
      gslc_ElemXTextboxScrollSet(pGui,pElemTmp,nVal,100);
      break;

    case E_SLIDER:
      // Fetch the slider position
      nVal = gslc_ElemXSliderGetPos(&m_gui,pElemRef);

      // Link slider to the numerical display
      snprintf(acTxt,20,(char*)"%u",nVal);
      pElemTmp = gslc_PageFindElemById(pGui,E_PG_MAIN,E_ELEM_TXT_COUNT);
      gslc_ElemSetTxtStr(pGui,pElemTmp,acTxt);

      // Link slider to insertion of text into textbox
      pElemTmp = gslc_PageFindElemById(pGui,E_PG_MAIN,E_ELEM_TEXTBOX);
      snprintf(acTxt,20,(char*)"Slider=%3u\n",nVal);
      gslc_ElemXTextboxAdd(pGui,pElemTmp,acTxt);

      break;

    default:
      break;
  }
  return true;
}

// Create page elements
bool InitOverlays()
{
  gslc_tsElemRef*  pElemRef = NULL;

  gslc_PageAdd(&m_gui,E_PG_MAIN,m_asPageElem,MAX_ELEM_PG_MAIN_RAM,m_asPageElemRef,MAX_ELEM_PG_MAIN);

  // Background flat color
  gslc_SetBkgndColor(&m_gui,GSLC_COL_GRAY_DK2);

  // Create Title with offset shadow
/*
  #define TMP_COL1 (gslc_tsColor){ 32, 32, 60}
  #define TMP_COL2 (gslc_tsColor){128,128,240}
  // Note: must use title Font ID
  gslc_ElemCreateTxt_P(&m_gui,98,E_PG_MAIN,2,2,320,50,"Home Automation",&m_asFont[1],
          TMP_COL1,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_MID,false,false);
  gslc_ElemCreateTxt_P(&m_gui,99,E_PG_MAIN,0,0,320,50,"Home Automation",&m_asFont[1],
          TMP_COL2,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_MID,false,false);
*/

  // Create background box
  gslc_ElemCreateBox_P(&m_gui,200,E_PG_MAIN,10,50,300,180,GSLC_COL_WHITE,GSLC_COL_BLACK,true,true,NULL,NULL);

  // Example horizontal slider
  pElemRef = gslc_ElemXSliderCreate(&m_gui,E_SLIDER,E_PG_MAIN,&m_sXSlider,
          (gslc_tsRect){20,60,140,20},0,100,50,5,false);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_GREEN,GSLC_COL_BLACK,GSLC_COL_BLACK);
  gslc_ElemXSliderSetStyle(&m_gui,pElemRef,true,GSLC_COL_GREEN_DK4,10,5,GSLC_COL_GRAY_DK2);
  gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbControls);

  // Text to show slider value
  pElemRef = gslc_ElemCreateTxt(&m_gui,E_ELEM_TXT_COUNT,E_PG_MAIN,(gslc_tsRect){180,60,40,20},
    (char*)"",0,E_FONT_TXT);


  // Create wrapping box for textbox and scrollbar
  pElemRef = gslc_ElemCreateBox(&m_gui,GSLC_ID_AUTO,E_PG_MAIN,(gslc_tsRect){18,83,203,124});
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_BLUE_DK4,GSLC_COL_BLACK,GSLC_COL_BLACK);

  // Create textbox
  pElemRef = gslc_ElemXTextboxCreate(&m_gui,E_ELEM_TEXTBOX,E_PG_MAIN,
    &m_sTextbox,(gslc_tsRect){20,85,180,120},E_FONT_TXT,(char*)&m_acTextboxBuf,
        TBOX_ROWS,TBOX_COLS);
  gslc_ElemXTextboxWrapSet(&m_gui,pElemRef,true);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_BLUE_LT2,GSLC_COL_BLACK,GSLC_COL_GRAY_DK3);
  m_pElemTextbox = pElemRef;

  // Create vertical scrollbar for textbox
  pElemRef = gslc_ElemXSliderCreate(&m_gui,E_SCROLLBAR,E_PG_MAIN,&m_sXSliderText,
        (gslc_tsRect){200,85,20,120},0,100,100,5,true);
  gslc_ElemSetCol(&m_gui,pElemRef,GSLC_COL_BLUE_DK4,GSLC_COL_BLACK,GSLC_COL_BLACK);
  gslc_ElemXSliderSetPosFunc(&m_gui,pElemRef,&CbControls);

  // Quit button
  gslc_ElemCreateBtnTxt_P(&m_gui,E_ELEM_BTN_QUIT,E_PG_MAIN,250,60,50,30,"Quit",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,&CbBtnQuit,NULL);

  return true;
}


void setup()
{
  bool bOk = true;

  // Initialize debug output
  Serial.begin(9600);
  gslc_InitDebug(&DebugOut);
  //delay(1000);  // NOTE: Some devices require a delay after Serial.begin() before serial port can be used

  // Initialize
  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }

  // Load Fonts
  // - NOTE: If we are using the ElemCreate*_P() macros then it is important to note
  //   the font pointer (array index) as it will be provided to certain
  //   ElemCreate*_P() functions (eg. ElemCreateTxt_P).
  if (!gslc_FontAdd(&m_gui,E_FONT_TXT,GSLC_FONTREF_PTR,NULL,1)) { return; }   // m_asFont[0]
  if (!gslc_FontAdd(&m_gui,E_FONT_TITLE,GSLC_FONTREF_PTR,NULL,3)) { return; } // m_asFont[1]

  // Create pages display
  InitOverlays();

  // Start up display on main page
  gslc_SetPageCur(&m_gui,E_PG_MAIN);

  // Insert some text
  gslc_tsElemRef* pElemTextbox = gslc_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_TEXTBOX);

  gslc_ElemXTextboxAdd(&m_gui,pElemTextbox,(char*)"Hi!\n");

  gslc_ElemXTextboxColSet(&m_gui,pElemTextbox,GSLC_COL_RED);
  gslc_ElemXTextboxAdd(&m_gui,pElemTextbox,(char*)"RED");
  gslc_ElemXTextboxColReset(&m_gui,pElemTextbox);
  gslc_ElemXTextboxAdd(&m_gui,pElemTextbox,(char*)"\n");
  gslc_ElemXTextboxAdd(&m_gui,pElemTextbox,(char*)"Long line here that might wrap\n");
  gslc_ElemXTextboxAdd(&m_gui,pElemTextbox,(char*)"Goodbye...\n");

  m_bQuit = false;
  return;
}

void loop()
{
    char                acTxt[MAX_STR];

  // General counter
  m_nCount++;

    if ((m_nCount % 5000) == 0) {
      snprintf(acTxt,MAX_STR,"%u\n",m_nCount);
      gslc_ElemXTextboxAdd(&m_gui,m_pElemTextbox,acTxt);
    }

  // Periodically call GUIslice update function
  gslc_Update(&m_gui);

  // In a real program, we would detect the button press and take an action.
  // For this Arduino demo, we will pretend to exit by emulating it with an
  // infinite loop. Note that interrupts are not disabled so that any debug
  // messages via Serial have an opportunity to be transmitted.
  if (m_bQuit) {
    gslc_Quit(&m_gui);
    while (1) { }
  }
}

