#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/modulemgr.h>
#include <psp2/ctrl.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/net.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/sysmodule.h>
#include <psp2/net/http.h>
#include <psp2/rtc.h>

#include <taihen.h>

/*
#define DEBUG_PORT 16000
int debug_sock = -1;
*/

SceUID hook[10];
static tai_hook_ref_t hook_ref[10];

/*
void init_log() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	
	int ret = sceNetShowNetstat();
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		SceNetInitParam initparam;
		initparam.memory = malloc(0x100000);
		if (initparam.memory == NULL){
			SceUID netblock = sceKernelAllocMemBlock("debugBlock", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 0x100000, NULL);
			if (netblock < 0) sceKernelExitDeleteThread(0);
			else sceKernelGetMemBlockBase(netblock, &initparam.memory);
		}
		initparam.size = 0x100000;
		initparam.flags = 0;
		sceNetInit(&initparam);
	}

	SceNetSockaddrIn server;
	server.sin_len = sizeof(server);
	server.sin_family = SCE_NET_AF_INET;
	sceNetInetPton(SCE_NET_AF_INET, "192.168.1.27", &server.sin_addr);
	server.sin_port = sceNetHtons(DEBUG_PORT);
	memset(server.sin_zero, 0, sizeof(server.sin_zero));

	debug_sock = sceNetSocket("debug_log", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	sceNetConnect(debug_sock, (SceNetSockaddr *)&server, sizeof(server));
}

void close_log() {
	sceNetCtlTerm();
	sceNetTerm();

	if (debug_sock) {
		sceNetSocketClose(debug_sock);
		debug_sock = -1;
	}
}

void log(char *text, ...) {
	va_list list;
	char string[512];

	va_start(list, text);
	vsprintf(string, text, list);
	va_end(list);

	sceNetSend(debug_sock, string, strlen(string), 0);
}

void log_hexdump(void* ptr, int size) {
	int nbr_loop = size / 16;
	log("%p———00—01—02—03—04—05—06—07—08—09—0A—0B—0C—0D—0E—0F———————————————————\n", ptr);
	for (int i = 0; i < nbr_loop; i++) {
		char* data = (char*)(ptr + (i*16));
		log("%p   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", data, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
	}
	log("———————END OF HEX DUMP———————\n");
}
*/

int in_statusbar = 0;

static int status_draw_time_patch(void *a1, int a2)
{
    in_statusbar = 1;
    int out = TAI_CONTINUE(int, hook_ref[0], a1, a2);
    in_statusbar = 0;
    return out;
}

static uint16_t **strdup_patch(uint16_t **a1, uint16_t *a2, int a2_size)
{
	if (in_statusbar) {
		SceDateTime time_local;
		SceDateTime time_utc;
		sceRtcGetCurrentClock(&time_utc, 0);

		SceRtcTick tick;
		sceRtcGetTick(&time_utc, &tick);
		sceRtcConvertUtcToLocalTime(&tick, &tick);
		sceRtcSetTick(&time_local, &tick);

		char buff[10];
		int len = sceClibSnprintf(buff, 10, ":%02d", time_local.second);
        for (int i = 0; i < len; ++i) {
            a2[a2_size + i] = buff[i];
        }

		return TAI_CONTINUE(uint16_t**, hook_ref[1], a1, a2, a2_size + len);
	}
	else {
		return TAI_CONTINUE(uint16_t**, hook_ref[1], a1, a2, a2_size);
	}	
}

/*
static int flightmode_patch(int unk0) {
	log("Called: FLIGHT MODE\n");
	return 0;
}

static int bluetooth_patch(int unk0) {
	log("Called: BLUETOOTH\n");
	return 0;
}

static int wlan_patch(int unk0) {
	log("Called: WLAN\n");
	return 0;
}

int strlen_spe(char* text) {
	int pos = 0;
	while (1) {
		if (text[pos] == 0x0 && text[pos+1] == 0x0) {
			break;
		}
		pos++;
	}
	return (pos / 2) + 1;
}

void log_str_spe(char* text) {
	int len = strlen_spe(text);
	for (int i = 0; i < (len*2); i++) {
		log("%c", text[i]);
	}

	log(" - size %i\n", len);
}

static wchar_t *scePafToplevelGetText_patch(void *arg, char **msg) {
  int ret = TAI_CONTINUE(wchar_t *, hook_ref[5], arg, msg);

  log("Arg:\n");
  log_hexdump(arg, 255);
  log("Msg (inside arg):\n");
  log_hexdump(*msg, 255);
  log("Msg:\n");
  log_hexdump(msg, 255);
  return ret;
}

static int ScePafToplevel_004D98CC_patch(char* plugin) {
	//int res = TAI_CONTINUE(int, &hook_ref[7], plugin);
	log("Plugin name: %s\n", plugin);
	return 0;
}

static int set_brightness_patch() {
	log("Call: BRIGHTNESS\n");
	return 0;
}
*/

int module_start(SceSize argc, const void *args) {
	//init_log();

    tai_module_info_t info;
    info.size = sizeof(info);
    int ret = taiGetModuleInfo("SceShell", &info);
    if (ret < 0) {
        //log("taiGetModuleInfo error: %X", ret);
        return SCE_KERNEL_START_FAILED;
    }

    //log("hooking ...\n");

    hook[0] = taiHookFunctionOffset(&hook_ref[0],
                                       info.modid,
                                       0,          // segidx
                                       0x183ea4, // offset
                                       1,          // thumb
                                       status_draw_time_patch);

    hook[1] = taiHookFunctionOffset(&hook_ref[1],
                                       info.modid,
                                       0,          // segidx
                                       0x40e0b4, // offset
                                       1,          // thumb
                                       strdup_patch);

    /*
    hook[2] = taiHookFunctionOffset(&hook_ref[2],
                                       info.modid,
                                       0,          // segidx
                                       0x15986C, // offset (flight mode)
                                       1,          // thumb
                                       flightmode_patch);

    hook[3] = taiHookFunctionOffset(&hook_ref[3],
                                       info.modid,
                                       0,          // segidx
                                       0x159B0A, // offset (wifi)
                                       1,          // thumb
                                       wlan_patch);

    hook[4] = taiHookFunctionOffset(&hook_ref[4],
                                       info.modid,
                                       0,          // segidx
                                       0x159B64, // offset (bluetooth)
                                       1,          // thumb
                                       bluetooth_patch);

	hook[5] = taiHookFunctionImport(&hook_ref[5], 
                                      TAI_MAIN_MODULE,
                                      TAI_ANY_LIBRARY,
                                      0x19CEFDA7,
                                      scePafToplevelGetText_patch);

    hook[6] = taiHookFunctionOffset(&hook_ref[6],
                                       info.modid,
                                       0,          // segidx
                                       0x14DE14, // offset (set_brightness (en faite c'est quand je lache))
                                       1,          // thumb
                                       set_brightness_patch);

	hook[7] = taiHookFunctionImport(&hook_ref[7], 
                                      TAI_MAIN_MODULE,
                                      TAI_ANY_LIBRARY,
                                      0x004D98CC,
                                      ScePafToplevel_004D98CC_patch);

	*/

	//  ScePafWidget_FF5BC957 => Une sorte de systéme de focus bizzare, sa ne marche que sur le menu principal (icone)
	// ScePafWidget_4588699E => A l'air de controllé la vitesse de d'animation de scroll (un lock peut-être ?) ou un clear plus rapide ? (quand j'ouvrai les dossier, il y avait un bug avec des icone qui resté figé 1s)
	// ScePafStdc_C95732BB => n'est jamais apelé en cours de lancement, mais d'aprés ida, il charge toute les resource néssesaire à l'affichage du livearea
	// ScePafWidget_FB7FE189 => semble ajouté un callback à un widget
	// ScePafWidget_065D3B50 => Sa à l'air très important (n'affiche plus rien pendant 30s, puis crash)
	// ScePafStdc_1B77082E => test

    //log("All hook activated !\n");

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	//close_log();
	return SCE_KERNEL_STOP_SUCCESS;
}

void _start() __attribute__ ((weak, alias ("module_start")));