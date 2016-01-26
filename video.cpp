#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"
extern "C" {
#include "ilclient.h"
}

static int video_decode_test(char *filename) {
  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
  OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
  COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
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

  if (OMX_Init() != OMX_ErrorNone) {
    ilclient_destroy(client);
    fclose(in);
    return -4;
  }

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
  configDisplay.set = (OMX_DISPLAYSETTYPE) (OMX_DISPLAY_SET_DEST_RECT | OMX_DISPLAY_SET_SRC_RECT | OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_NOASPECT);
  configDisplay.dest_rect.x_offset = 100;
  configDisplay.dest_rect.y_offset = 100;
  configDisplay.dest_rect.width = 640;
  configDisplay.dest_rect.height = 480;

  OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexConfigDisplayRegion, &configDisplay);

  set_tunnel(tunnel, video_decode, 131, video_render, 90);

  if (status == 0)
    ilclient_change_component_state(video_decode, OMX_StateIdle);

  memset(&format, 0, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
  format.nSize = sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE);
  format.nVersion.nVersion = OMX_VERSION;
  format.nPortIndex = 130;
  format.eCompressionFormat = OMX_VIDEO_CodingAVC;

  if (status == 0 &&
          OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
          ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0) {
    OMX_BUFFERHEADERTYPE *buf;
    int port_settings_changed = 0;
    int first_packet = 1;

    ilclient_change_component_state(video_decode, OMX_StateExecuting);

    while ((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL) {
      // feed data and wait until we get port settings changed
      unsigned char *dest = buf->pBuffer;

      data_len += fread(dest, 1, buf->nAllocLen - data_len, in);

      if (port_settings_changed == 0 &&
              ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
              (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
              ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {
        port_settings_changed = 1;

        if (ilclient_setup_tunnel(tunnel, 0, 0) != 0) {
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

int main(int argc, char **argv) {
  bcm_host_init();
  return video_decode_test("video-LQ.h264");
}


