//=================================================================================================

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "PDNA.h"

// Set OVERRIDE_BAUD to 1 to program custom bit rate
#define OVERRIDE_BAUD 0
// Set CHAR_DELAY to 1 to introduce a delay between sent characters
#define CHAR_DELAY 0

// channel configuration helper macros
#define CFG_508(OPER, MODE, BAUD, WIDTH, STOP, PARITY, ERROR) \
    (OPER << DQ_SL501_OPER_SH | MODE << DQ_SL501_MODE_SH | BAUD << DQ_SL501_BAUD_SH | \
    WIDTH << DQ_SL501_WIDTH_SH | STOP  << DQ_SL501_STOP_SH | PARITY << DQ_SL501_PARITY_SH | \
    ERROR << DQ_SL501_ERROR_SH)

#define CFG_485F CFG_508(DQ_SL501_OPER_NORM, DQ_SL501_MODE_485F, DQ_SL501_BAUD_9600, \
    DQ_SL501_WIDTH_8, DQ_SL501_STOP_1, DQ_SL501_PARITY_NONE, 0)

#define CFG_485H CFG_508(DQ_SL501_OPER_NORM, DQ_SL501_MODE_485H, DQ_SL501_BAUD_9600, \
    DQ_SL501_WIDTH_8, DQ_SL501_STOP_1, DQ_SL501_PARITY_NONE, 0)

#if OVERRIDE_BAUD
#define CFG_232 CFG_508(DQ_SL501_OPER_NORM, DQ_SL501_MODE_232, DQ_SL501_BAUD_CUST, \
    DQ_SL501_WIDTH_8, DQ_SL501_STOP_1, DQ_SL501_PARITY_NONE, 0)
#else
#define CFG_232 CFG_508(DQ_SL501_OPER_NORM, DQ_SL501_MODE_232, DQ_SL501_BAUD_9600, \
    DQ_SL501_WIDTH_8, DQ_SL501_STOP_1, DQ_SL501_PARITY_NONE, 0)
#endif

/* BEGIN CUSTOM CONFIGURATION */

// IP address of cube
#define IOM_IPADDR0     "192.168.0.42"

// Layer to work with
#define DEVN           3

// Channels direction
#define CHAN0          (0) // transceiver
#define CHAN1          (1) // transceiver
#define CHAN2          (2) // transceiver
#define CHAN3          (3) // transceiver
#define CHAN4          (4) // transceiver
#define CHAN5          (5) // transceiver

int channelList[] = { CHAN0,CHAN1,CHAN2,CHAN3,CHAN4,CHAN5 };

#define SLEEP_TIME     2000
// Cahannels configuration
#define CHANNEL_CFG (CFG_232) // change this to test 485F/485H/RS232

/* END CUSTOM CONFIGURATION */

#define TIMEOUT_DELAY   1000  // milliseconds
#define SYMNUM  (8)

//int stop;

bool CHASIS_INFO=false;
bool ALL_CHANNELS=false;
bool OPEN_CHANNELS=false;
bool CLOSE_CHANNELS=false;
bool RESET_CHANNELS=false;
bool HELP=false;
bool Kanal_Listesi_Acik[]={false,false,false,false,false,false};
bool Kanal_Listesi_Kapali[]={false,false,false,false,false,false};
bool Kanal_Listesi_AcikKapali[]={false,false,false,false,false,false};

char chrCR='\r';
char chrLF='\n';

int Parametreler(int argc, char* argv[]);
void ShowHelp();
int ChannelsAll();
int ChannelsOpenClose(int IsOpen);
int ChannelsReset();
int ConnectIOChasis();
int GetDevInfo();
int SendCmd(char cmd[],int chan);
void ShowGrid();
void ShowChanStat();
void gotoxy();

int hd = 0;
ushort RowSize;
ushort ColSize;

// ----------------- main routine -----------------------------
//argv degerleri: /i                                    -> Cihaz bilgisi
//                /a                                    -> Tüm kanalları açıp kapat
//                /o Kanal_No0[,Kanal_No1,Kana_No2...]  -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) Aç
//                /c Kanal_No0[,Kanal_No1,Kana_No2...]  -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) Kapat
//                /r Kanal_No0[,Kanal_No1,Kana_No2...] -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) önce Aç sonra Kapat
//argv degerleri: /h                                    -> Yardım


int main(int argc, char* argv[]) {
    //getchar();
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    RowSize=w.ws_row;
    ColSize=w.ws_col;

    ShowGrid();
    /*
    ShowChanStat(0,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(0,0);
    ShowChanStat(1,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(1,0);
    ShowChanStat(2,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(2,0);
    ShowChanStat(3,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(3,0);
    ShowChanStat(4,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(4,0);
    ShowChanStat(5,1);
    fflush(stdout);
    sleep(1);
    ShowChanStat(5,0);
    fflush(stdout);
    sleep(1);

    return 0;
    */

    int sonuc=0;
    DQRDCFG *dqr=NULL;

    Parametreler(argc,argv);

    /*
    printf("CHI: %i\n",(int)CHASIS_INFO);
    printf("ALL: %i\n",(int)ALL_CHANNELS);
    printf("OPN: %i\n",(int)OPEN_CHANNELS);
    printf("CLS: %i\n",(int)CLOSE_CHANNELS);
    printf("RST: %i\n",(int)RESET_CHANNELS);
    printf("HLP: %i\n",(int)HELP);
    */

    /*for(int i=0;i<6;i++)
        printf("%i , %i\n",i,Kanal_Listesi_Acik[i]);*/

    if(HELP)
        ShowHelp();
    else
        ShowGrid();

    if(ALL_CHANNELS){
        printf("*** Tüm PSU'lar Resetleniyor ***\n");
        sonuc=ChannelsAll();
    }

    if(OPEN_CHANNELS){
        printf("*** Seçilen PSU'lar Açılıyor ***\n");
        sonuc=ChannelsOpenClose(true);
    }

    if(CLOSE_CHANNELS){
        printf("*** Seçilen PSU'lar Kapatılıyor ***\n");
        sonuc=ChannelsOpenClose(false);
    }

    if(RESET_CHANNELS){
        printf("*** Seçilen PSU'lar Resetleniyor ***\n");
        sonuc=ChannelsReset();
    }

    if(CHASIS_INFO){
        if(ConnectIOChasis(dqr)){
            GetDevInfo(dqr);
        }
    }
return sonuc;
}


int Parametreler(int argc, char* argv[]){
    for(int i=0;i<argc;i++){
        if(strncmp(argv[i],"/i",2)==0)
            CHASIS_INFO=true;

        if(strncmp(argv[i],"/a",2)==0)
            ALL_CHANNELS=true;

        if(strncmp(argv[i],"/o",2)==0){
            OPEN_CHANNELS=true;
            for(int j=i+1;j<argc;j++)
                if(strchr(argv[j],'/')==NULL)
                    Kanal_Listesi_Acik[atoi(argv[j])]=true;
                else
                    break;
        }

        if(strncmp(argv[i],"/c",2)==0){
            CLOSE_CHANNELS=true;
            for(int j=i+1;j<argc;j++)
                if(strchr(argv[j],'/')==NULL)
                    Kanal_Listesi_Kapali[atoi(argv[j])]=true;
                else
                    break;
        }

        if(strncmp(argv[i],"/r",2)==0){
            RESET_CHANNELS=true;
            for(int j=i+1;j<argc;j++)
                if(strchr(argv[j],'/')==NULL)
                    Kanal_Listesi_AcikKapali[atoi(argv[j])]=true;
                else
                    break;
        }

        if(strncmp(argv[i],"/h",2)==0)
            HELP=true;
    }
return 0;
}


void ShowHelp(){
    printf("_______________________________________________________ YARDIM ____________________________________________________\n\n");
    printf("Program: PSC (Power Supply Control)\n");
    printf("Yazan..: Süleyman GÜNEL, 2020 (c)\n\n");
    printf("/i                                    -> Cihaz bilgisi\n");
    printf("/a                                    -> Tüm kanalları açıp kapat\n");
    printf("/o Kanal_No0[,Kanal_No1,Kana_No2...]  -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) Aç\n");
    printf("/c Kanal_No0[,Kanal_No1,Kana_No2...]  -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) Kapat\n");
    printf("/r Kanal_No0[,Kanal_No1,Kana_No2...]  -> Kanal_No0... numaralı kanalı/kanalları (0 ilk kanaldır) önce Aç sonra Kapat\n");
    printf("/h                                    -> Yardım\n\n");
}


int ChannelsAll(){
    int sonuc=0;
    for(int i=0;i<=5;i++){
        sonuc=SendCmd("ADR 6",channelList[i]);
        sonuc=SendCmd("OUT 1",channelList[i]);
    }
    for(int i=0;i<=5;i++){
        sonuc=SendCmd("OUT 0",channelList[i]);
    }
    return sonuc;
}

int ChannelsOpenClose(int IsOpen){
    int sonuc=0;
    for(int i=0;i<=5;i++){
        if(IsOpen?Kanal_Listesi_Acik[i]:Kanal_Listesi_Kapali[i]){
            sonuc=SendCmd("ADR 6",channelList[i]);
            if(IsOpen)
                sonuc=SendCmd("OUT 1",channelList[i]);
            else
                sonuc=SendCmd("OUT 0",channelList[i]);
        }
    }
    return sonuc;
}

int ChannelsReset(){
    int sonuc=0;
    for(int i=0;i<=5;i++){
        if(Kanal_Listesi_AcikKapali[i]){
            sonuc=SendCmd("ADR 6",channelList[i]);
            sonuc=SendCmd("OUT 1",channelList[i]);
            sonuc=SendCmd("OUT 0",channelList[i]);
        }
    }
    return sonuc;
}

int ConnectIOChasis(){
    DQRDCFG *DQRdCfg = NULL;
    int ret;
    int channelCount;
    int i;
    //int msgCount = 0;
    //int avail;
    //FILE* fo = NULL;
    //uint16 boy=7;
    //char str[boy];
    //strncpy(str,"ADR 6\r\n",boy);
    //uint16 size;
    //uint16 len=boy;
    //uint8 data[DQ_MAX_PKT_SIZE];
    //uint8 error[DQ_MAX_PKT_SIZE];
    //int success;

#if OVERRIDE_BAUD
    int realbaud;
#endif

    DqInitDAQLib();

    // Open communication with IOM and receive IOM crucial identification data
    Chk4Err(DqOpenIOM(IOM_IPADDR0, DQ_UDP_DAQ_PORT, TIMEOUT_DELAY, &hd, &DQRdCfg), goto finish_up);

    if (DQRdCfg == NULL) {
        printf("\nError in receiving the response for Echo Command");
        goto finish_up;
    }

    //Chasis info istenirse dqr üzerinden veriler main'e aktarılacak.
    //*dqr = *DQRdCfg;*/

     //verify board is in configurable state
    ChkOpsMode(hd, DEVN, goto finish_up);

    channelCount = sizeof(channelList)/sizeof(int);

    // set channels configuration
    for (i = 0; i < channelCount; i++) {
        // set internal base clock frequency (66MHz by default); can also be DQ_L501_BASE_24
        Chk4Err(DqAdv508BaseClock(hd, DEVN, channelList[i], DQ_L501_BASE_66), goto finish_up);

        // set channels configuration
        Chk4Err(DqAdv501SetChannelCfg(hd, DEVN, channelList[i], CHANNEL_CFG), goto finish_up);

#if OVERRIDE_BAUD
        // override baud settings
        Chk4Err(DqAdv501SetBaud(hd, DEVN, channelList[i], 9600, &realbaud), goto finish_up);
#endif

#if CHAR_DELAY
        // introduce character delay
        Chk4Err(DqAdv501SetCharDelay(hd, DEVN, channelList[i], DQ_SL501_DELAYMODE_INTERNAL, 10000), goto finish_up);
#endif

        Chk4Err(DqAdv501SetTimeout(hd, DEVN, channelList[i], 1000), goto finish_up);
        Chk4Err(DqAdv501SetTermLength(hd, DEVN, channelList[i], SYMNUM), goto finish_up);
        Chk4Err(DqAdv501SetWatermark(hd, DEVN, channelList[i], DQL_IOCTL501_SETTXWM, 1), goto finish_up);
    }
    Chk4Err(DqAdv501Enable(hd, DEVN, TRUE), goto finish_up);

        //printf("Cihaz Bağlantısı Başarılı\n");
        return 0;

finish_up:

    Chk4Err(DqAdv501Enable(hd, DEVN, FALSE), );
    if (hd) {
       Chk4Err(DqCloseIOM(hd), );
    }
    DqCleanUpDAQLib();
    //printf("Cihaz Bağlantısı Başarısız !\n");
    return 1;
}


int GetDevInfo(DQRDCFG *dqr){
    printf("ipaddr  = %d.%d.%d.%d\n",(dqr->ipaddr & 0xff000000)>>24,(dqr->ipaddr & 0x00ff0000)>>16,(dqr->ipaddr & 0x0000ff00)>>8,(dqr->ipaddr & 0x000000ff));
    printf("model   = %04x\n", dqr->model);
    printf("sernum  = %07d\n", dqr->sernum);
    printf("mfgdate = %x/%x/%x\n", (dqr->mfgdate & 0xff000000)>>24,(dqr->mfgdate & 0xff0000)>>16,(dqr->mfgdate & 0xffff));
    printf("caldate = %x/%x/%x\n", (dqr->caldate & 0xff000000)>>24,(dqr->caldate & 0xff0000)>>16,(dqr->caldate & 0xffff));
    return(0);
}


int SendCmd(char cmd[],int chan){
    //sleep(5);
    if(ConnectIOChasis())
        return 1;
    int ret;
    uint16 boy=strlen(cmd);
    char _cmd[boy+2];
    strcpy(_cmd,cmd);
    _cmd[boy]=chrCR;
    _cmd[boy+1]=chrLF;
    boy=strlen(_cmd);
    uint16 size;
    uint8 error[DQ_MAX_PKT_SIZE];
    int success;
    int hata=0;

    //printf("Channel:%i, Command:%s",chan,cmd);
    //printf("\n");

    Chk4Err(DqAdv501SendMessage(hd, DEVN, chan, (uint8*)_cmd, boy, &size, &success, error), hata=1);
    UeiPalSleep(SLEEP_TIME);
    Chk4Err(DqAdv501Enable(hd, DEVN, FALSE), );
    if(hd)
        Chk4Err(DqCloseIOM(hd), );
    DqCleanUpDAQLib();
    if(hata || boy!=size){
        printf("Hata Oldu: %s",error);
        hata=1;
    }
    //if(hata==0)
        ShowChanStat(chan,(strncmp(cmd,"OUT 1",5)==0?1:0));
    return hata;
}

void ShowGrid(){
    system("clear");
    gotoxy(ColSize/2-12,RowSize/2-3);
    printf("KANAL DURUMU (AÇIK/KAPALI)");
    gotoxy(ColSize/2-18,RowSize/2-2);
    printf("╔═════╦═════╦═════╦═════╦═════╦═════╗");printf("\n");
    gotoxy(ColSize/2-18,RowSize/2-1);
    printf("║ CH0 ║ CH1 ║ CH2 ║ CH3 ║ CH4 ║ CH5 ║");printf("\n");
    gotoxy(ColSize/2-18,RowSize/2);
    printf("╠═════╬═════╬═════╬═════╬═════╬═════╣");printf("\n");
    gotoxy(ColSize/2-18,RowSize/2+1);
    printf("║     ║     ║     ║     ║     ║     ║");printf("\n");
    gotoxy(ColSize/2-18,RowSize/2+2);
    printf("╚═════╩═════╩═════╩═════╩═════╩═════╝");printf("\n");
}

void ShowChanStat(int chan,int stat){
    //printf("%i",stat);
    gotoxy((chan+1)*6+(ColSize/2-22),RowSize/2+1);
    stat?printf("███"):printf("   ");
    gotoxy(0,RowSize-1);
    fflush(stdout);
}

void gotoxy(int x,int y)
{
    printf("%c[%d;%df",0x1B,y,x);
}

