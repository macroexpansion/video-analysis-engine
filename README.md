===============================================================================
1. Prerequisites:
===============================================================================

Please follow instructions in the apps/sample_apps/deepstream-app/README on how
to install the prerequisites for the Deepstream SDK, the DeepStream SDK itself,
and the apps.

You must have the following development packages installed
   GStreamer-1.0
   GStreamer-1.0 Base Plugins
   GStreamer-1.0 gstrtspserver
   X11 client-side library

To install these packages, execute the following command:
   sudo apt-get install libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev \
   libgstrtspserver-1.0-dev libx11-dev

===============================================================================
2. Purpose:
===============================================================================

This document describes the sample deepstream-test3 application.

This sample builds on top of the deepstream-test1 sample to demonstrate how to:

* Use multiple sources in the pipeline.
* Use a uridecodebin so that any type of input (e.g. RTSP/File), any GStreamer
  supported container format, and any codec can be used as input.
* Configure the stream-muxer to generate a batch of frames and infer on the
  batch for better resource utilization.
* Extract the stream metadata, which contains useful information about the
  frames in the batched buffer.

Refer to the deepstream-test1 sample documentation for an example of a
single-stream inference, bounding-box overlay, and rendering.

===============================================================================
3. To compile:
===============================================================================

  $ Set CUDA_VER in the MakeFile as per platform.
      For Jetson, CUDA_VER=11.4
      For x86, CUDA_VER=11.7
  $ sudo make

NOTE: To compile the sources, run make with "sudo" or root permission.

===============================================================================
4. Usage:
===============================================================================

  Two ways to run the application:

  1.Run with the uri(s). In this method, user needs to modify the source
    code of deepstream-test3-app to configure pipeline properties.

    $ ./deepstream-test3-app <uri1> [uri2] ... [uriN]
    e.g.
    $ ./deepstream-test3-app file:///home/ubuntu/video1.mp4 file:///home/ubuntu/video2.mp4
    $ ./deepstream-test3-app rtsp://127.0.0.1/video1 rtsp://127.0.0.1/video2

  2.Run with the yml file. In this method, user needs to update the yml file to configure
    pipeline properties.

    $ ./deepstream-test3-app <yml file>
    e.g. $ ./deepstream-test3-app dstest3_config.yml

This sample accepts one or more H.264/H.265 video streams as input. It creates
a source bin for each input and connects the bins to an instance of the
"nvstreammux" element, which forms the batch of frames. The batch of
frames is fed to "nvinfer" for batched inferencing. The batched buffer is
composited into a 2D tile array using "nvmultistreamtiler." The rest of the
pipeline is similar to the deepstream-test1 sample.

The "width" and "height" properties must be set on the stream-muxer to set the
output resolution. If the input frame resolution is different from
stream-muxer's "width" and "height", the input frame will be scaled to muxer's
output resolution.

The stream-muxer waits for a user-defined timeout before forming the batch. The
timeout is set using the "batched-push-timeout" property. If the complete batch
is formed before the timeout is reached, the batch is pushed to the downstream
element. If the timeout is reached before the complete batch can be formed
(which can happen in case of rtsp sources), the batch is formed from the
available input buffers and pushed. Ideally, the timeout of the stream-muxer
should be set based on the framerate of the fastest source. It can also be set
to -1 to make the stream-muxer wait infinitely.

The "nvmultistreamtiler" composite streams based on their stream-ids in
row-major order (starting from stream 0, left to right across the top row, then
across the next row, etc.).

NOTE: To reuse engine files generated in previous runs, update the
model-engine-file parameter in the nvinfer config file to an existing engine file
