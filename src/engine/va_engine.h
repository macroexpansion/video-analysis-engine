#ifndef VA_ENGINE_ENGINE_H_
#define VA_ENGINE_ENGINE_H_

#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#include <iostream>
#include <stdexcept>

#include <gst/gst.h>
#include <glib.h>
#include <cuda_runtime_api.h>
#include "gstnvdsmeta.h"
#include "nvds_yml_parser.h"
#include "gst-nvmessage.h"

#include "va_database.h"
#include "va_user_data.h"

#define MAX_DISPLAY_LEN 64

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

/* By default, OSD process-mode is set to CPU_MODE. To change mode, set as:
 * 1: GPU mode (for Tesla only)
 * 2: HW mode (For Jetson only)
 */
#define OSD_PROCESS_MODE 1

/* By default, OSD will not display text. To display text, change this to 1 */
#define OSD_DISPLAY_TEXT 1

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 40000

#define TILED_OUTPUT_WIDTH 1280
#define TILED_OUTPUT_HEIGHT 720

/* NVIDIA Decoder source pad memory feature. This feature signifies that source
 * pads having this capability will push GstBuffers containing cuda buffers. */
#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

namespace va {
/**
 * Video analytic engine using GStreamer and TensorRT
 */
struct Engine {
	GMainLoop* m_loop;
	GstElement* m_pipeline;
	GstElement* m_streammux;
	GstElement* m_sink;
	GstElement* m_nvinfer;
	GstElement* m_queue1;
	GstElement* m_queue2;
	GstElement* m_queue3;
	GstElement* m_queue4;
	GstElement* m_queue5;
	GstElement* m_nvvidconv;
	GstElement* m_nvosd;
	GstElement* m_tiler;
	GstElement* m_nvdslogger;
	GstElement* m_transform;
	guint m_bus_watch_id;
	guint m_i = 0, m_num_sources = 0;
	guint m_tiler_rows, m_tiler_columns;
	guint m_nvinfer_batch_size;
	struct cudaDeviceProp m_cuda_prop;
	va::Database* m_va_database;

	int m_argc;
	char** m_argv;

	Engine(int argc, char** argv);
	~Engine();

	auto m_create_pipeline() -> GstElement*;
	auto m_create_streamux() -> GstElement*;
	auto m_create_source_bin(guint index, gchar* uri) -> GstElement*;
	auto m_add_source_bin_to_pipeline() -> void;
	auto m_create_nvinfer() -> GstElement*;
	auto m_create_queue() -> std::tuple<GstElement*, GstElement*, GstElement*, GstElement*, GstElement*>;
	auto m_create_nvdslogger() -> GstElement*;
	auto m_create_tiler() -> GstElement*;
	auto m_create_nvvidconv() -> GstElement*;
	auto m_create_nvvidosd() -> GstElement*;
	auto m_create_transform() -> GstElement*;
	auto m_create_sink() -> GstElement*;
	auto m_setup_element_config() -> void;
	auto m_add_elements_to_pipeline() -> void;
	auto m_create_message_handler() -> guint;
	auto m_add_tiler_src_pad_buffer_probe(va::UserData* va_user_data) -> void;

	auto run() -> void;
	auto set_database(va::Database* _va_database) -> void;
};
} // namespace va

#endif
