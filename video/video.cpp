#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <boost/thread.hpp>

#include "bcm_host.h"
extern "C" {
#include "ilclient.h"
}

static char const *err2str(int err) {
  switch (err) {
    case OMX_ErrorInsufficientResources: return "OMX_ErrorInsufficientResources";
    case OMX_ErrorUndefined: return "OMX_ErrorUndefined";
    case OMX_ErrorInvalidComponentName: return "OMX_ErrorInvalidComponentName";
    case OMX_ErrorComponentNotFound: return "OMX_ErrorComponentNotFound";
    case OMX_ErrorInvalidComponent: return "OMX_ErrorInvalidComponent";
    case OMX_ErrorBadParameter: return "OMX_ErrorBadParameter";
    case OMX_ErrorNotImplemented: return "OMX_ErrorNotImplemented";
    case OMX_ErrorUnderflow: return "OMX_ErrorUnderflow";
    case OMX_ErrorOverflow: return "OMX_ErrorOverflow";
    case OMX_ErrorHardware: return "OMX_ErrorHardware";
    case OMX_ErrorInvalidState: return "OMX_ErrorInvalidState";
    case OMX_ErrorStreamCorrupt: return "OMX_ErrorStreamCorrupt";
    case OMX_ErrorPortsNotCompatible: return "OMX_ErrorPortsNotCompatible";
    case OMX_ErrorResourcesLost: return "OMX_ErrorResourcesLost";
    case OMX_ErrorNoMore: return "OMX_ErrorNoMore";
    case OMX_ErrorVersionMismatch: return "OMX_ErrorVersionMismatch";
    case OMX_ErrorNotReady: return "OMX_ErrorNotReady";
    case OMX_ErrorTimeout: return "OMX_ErrorTimeout";
    case OMX_ErrorSameState: return "OMX_ErrorSameState";
    case OMX_ErrorResourcesPreempted: return "OMX_ErrorResourcesPreempted";
    case OMX_ErrorPortUnresponsiveDuringAllocation: return "OMX_ErrorPortUnresponsiveDuringAllocation";
    case OMX_ErrorPortUnresponsiveDuringDeallocation: return "OMX_ErrorPortUnresponsiveDuringDeallocation";
    case OMX_ErrorPortUnresponsiveDuringStop: return "OMX_ErrorPortUnresponsiveDuringStop";
    case OMX_ErrorIncorrectStateTransition: return "OMX_ErrorIncorrectStateTransition";
    case OMX_ErrorIncorrectStateOperation: return "OMX_ErrorIncorrectStateOperation";
    case OMX_ErrorUnsupportedSetting: return "OMX_ErrorUnsupportedSetting";
    case OMX_ErrorUnsupportedIndex: return "OMX_ErrorUnsupportedIndex";
    case OMX_ErrorBadPortIndex: return "OMX_ErrorBadPortIndex";
    case OMX_ErrorPortUnpopulated: return "OMX_ErrorPortUnpopulated";
    case OMX_ErrorComponentSuspended: return "OMX_ErrorComponentSuspended";
    case OMX_ErrorDynamicResourcesUnavailable: return "OMX_ErrorDynamicResourcesUnavailable";
    case OMX_ErrorMbErrorsInFrame: return "OMX_ErrorMbErrorsInFrame";
    case OMX_ErrorFormatNotDetected: return "OMX_ErrorFormatNotDetected";
    case OMX_ErrorContentPipeOpenFailed: return "OMX_ErrorContentPipeOpenFailed";
    case OMX_ErrorContentPipeCreationFailed: return "OMX_ErrorContentPipeCreationFailed";
    case OMX_ErrorSeperateTablesUsed: return "OMX_ErrorSeperateTablesUsed";
    case OMX_ErrorTunnelingUnsupported: return "OMX_ErrorTunnelingUnsupported";
    default: return "unknown error";
  }
}

static void error_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
  fprintf(stderr, "OMX error %s\n", err2str(data));
}

static int video_decode_test(int i, const char *filename, int x_offset, int y_offset, int width, int height) {
  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
  COMPONENT_T *video_decode = NULL, *video_render = NULL;
  COMPONENT_T * list[3];
  TUNNEL_T tunnel[2];
  ILCLIENT_T *client;
  FILE *in;
  int status = 0;
  unsigned int data_len = 0;

  memset(list, 0, sizeof (list));
  memset(tunnel, 0, sizeof (tunnel));

  if ((in = fopen(filename, "rb")) == NULL)
    return -2;

  if ((client = ilclient_init()) == NULL) {
    fclose(in);
    return -3;
  }

  ilclient_set_error_callback(client, error_callback, NULL);

  // create video_decode
  if (ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T) (ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
    status = -14;
  list[0] = video_decode;

  // create video_render
  if (status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
    status = -14;
  list[1] = video_render;

  // Set up ROI
  OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
  memset(&configDisplay, 0, sizeof (OMX_CONFIG_DISPLAYREGIONTYPE));
  configDisplay.nSize = sizeof (OMX_CONFIG_DISPLAYREGIONTYPE);
  configDisplay.nVersion.nVersion = OMX_VERSION;
  configDisplay.nPortIndex = 90;
  configDisplay.fullscreen = OMX_FALSE;
  configDisplay.noaspect = OMX_TRUE;
  configDisplay.set = (OMX_DISPLAYSETTYPE) (OMX_DISPLAY_SET_DEST_RECT | OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_NOASPECT);
  configDisplay.dest_rect.x_offset = x_offset;
  configDisplay.dest_rect.y_offset = y_offset;
  configDisplay.dest_rect.width = width;
  configDisplay.dest_rect.height = height;

  if (OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexConfigDisplayRegion, &configDisplay) != OMX_ErrorNone) {
    printf("OMX_GetParameter failed\n");
  }

  set_tunnel(tunnel, video_decode, 131, video_render, 90);

  if (status == 0)
    ilclient_change_component_state(video_decode, OMX_StateIdle);


  memset(&format, 0, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
  format.nSize = sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE);
  format.nVersion.nVersion = OMX_VERSION;
  format.nPortIndex = 130;
  format.eCompressionFormat = OMX_VIDEO_CodingAVC;

  if (OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) != OMX_ErrorNone) {
    printf("OMX_SetParameter OMX_IndexParamVideoPortFormat failed\n");
  }

  OMX_PARAM_PORTDEFINITIONTYPE portdef;
  portdef.nSize = sizeof (OMX_PARAM_PORTDEFINITIONTYPE);
  portdef.nVersion.nVersion = OMX_VERSION;
  portdef.nPortIndex = 130;
  if (OMX_GetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamPortDefinition, &portdef) == OMX_ErrorNone) {
    printf("OMX_GetParameter\n");
    portdef.nBufferCountActual = portdef.nBufferCountMin;
    portdef.nBufferSize = 32 * 1024;
    if (OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamPortDefinition, &portdef) != OMX_ErrorNone) {
      printf("OMX_SetParameter error\n");
    }
  }

  printf("%d ilclient_enable_port_buffers\n", i);
  if (status == 0 && ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0) {
    OMX_BUFFERHEADERTYPE *buf;
    int port_settings_changed = 0;
    int first_packet = 1;

    printf("%d ilclient_change_component_state\n", i);
    ilclient_change_component_state(video_decode, OMX_StateExecuting);

    printf("%d ilclient_get_input_buffer\n", i);
    while ((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL) {
      // feed data and wait until we get port settings changed
      unsigned char *dest = buf->pBuffer;

      data_len += fread(dest, 1, buf->nAllocLen - data_len, in);

      if (port_settings_changed == 0 &&
              ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
              (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
              ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {
        port_settings_changed = 1;
        printf("%d port_settings_changed\n", i);
        if (ilclient_setup_tunnel(tunnel, 0, 0) != 0) {
          printf("%d ilclient_setup_tunnel failed\n", i);
          status = -7;
          break;
        }

        ilclient_change_component_state(video_render, OMX_StateExecuting);
      }
      if (!data_len)
        break;

      buf->nFilledLen = data_len;
      data_len = 0;

      buf->nOffset = 0;
      if (first_packet) {
        buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
        first_packet = 0;
      } else
        buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

      if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone) {
        printf("%d OMX_EmptyThisBuffer failed\n", i);
        status = -6;
        break;
      }
    }

    buf->nFilledLen = 0;
    buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

    if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
      status = -20;

    // wait for EOS from render
    ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
            ILCLIENT_BUFFER_FLAG_EOS, 10000);

    // need to flush the renderer to allow video_decode to disable its input port
    ilclient_flush_tunnels(tunnel, 0);

  }

  fclose(in);

  ilclient_disable_tunnel(tunnel);
  ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
  ilclient_teardown_tunnels(tunnel);

  ilclient_state_transition(list, OMX_StateIdle);
  ilclient_state_transition(list, OMX_StateLoaded);

  ilclient_cleanup_components(list);

  OMX_Deinit();

  ilclient_destroy(client);
  return status;
}

int main() {
  bcm_host_init();


  if (OMX_Init() != OMX_ErrorNone) {
    return -4;
  }

  int width = 1920;
  int height = 1080;
  int cols = 4;
  int rows = 3;

  boost::thread_group g;

  int i = 1;
  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      int xoff = x * (width / cols);
      int yoff = y * (height / rows);

      g.create_thread(boost::bind(video_decode_test, i, "video-LQ-640.h264", xoff, yoff, width / cols, height / rows));
      usleep(1000 * 100);
      i++;
    }
  }
  g.join_all();
}


