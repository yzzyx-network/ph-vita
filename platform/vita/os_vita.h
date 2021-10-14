/*************************************************************************/
/*  os_vita.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef OS_VITA_H
#define OS_VITA_H

#include "context_egl_vita.h"
#include "drivers/dummy/texture_loader_dummy.h"
#include "drivers/unix/os_unix.h"
#include "core/os/os.h"
#include "main/input_default.h"
#include "servers/visual/visual_server_raster.h"

#include <psp2/appmgr.h>
#include <psp2/kernel/processmgr.h>
#include "joypad_vita.h"

class OS_Vita : public OS {
    MainLoop *main_loop;
    virtual void delete_main_loop();
    ContextEGL_Vita *gl_context;
    VideoMode current_videomode;
    VisualServer *visual_server;
    InputDefault *input;
    JoypadVita *joypad;
    Vector2 last_touch_pos[SCE_TOUCH_MAX_REPORT];
    SceTouchData touch;
    SceTouchPanelInfo front_panel_info;
    Vector2 front_panel_size;
    void process_touch();
    int video_driver_index;
protected:
	virtual void initialize_core();
	virtual Error initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver);
	virtual void finalize();
    virtual void finalize_core();

	virtual void set_main_loop(MainLoop *p_main_loop);
public:
    virtual bool _check_internal_feature_support(const String &p_feature);
    virtual void alert(const String &p_alert, const String &p_title = "ALERT!");

    virtual Point2 get_mouse_position() const;
    virtual int get_mouse_button_state() const;
    virtual void set_window_title(const String &p_title);

    virtual void set_video_mode(const VideoMode &p_video_mode, int p_screen);
    virtual VideoMode get_video_mode(int p_screen) const;
    virtual void get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const;
    OS::RenderThreadMode get_render_thread_mode() const;
	virtual bool can_draw() const;
    virtual int get_current_video_driver() const;
    virtual Size2 get_window_size() const;

    virtual Error execute(const String &p_path, const List<String> &p_arguments, bool p_blocking = true, int64_t *r_child_id = NULL, String *r_pipe = NULL, int *r_exitcode = NULL, bool read_stderr = false, Mutex *p_pipe_mutex = NULL);
    virtual Error kill(const ProcessID &p_pid);

    virtual String get_name() const;
    void run();
    virtual void swap_buffers();
    virtual MainLoop *get_main_loop() const;

    virtual bool has_environment(const String &p_var) const;
    virtual String get_environment(const String &p_var) const;
    virtual bool set_environment(const String &p_var, const String &p_value) const;

    virtual Date get_date(bool local = false) const;
    virtual Time get_time(bool local = false) const;
    virtual TimeZoneInfo get_time_zone_info() const;
    virtual void delay_usec(uint32_t p_usec) const;
    virtual uint64_t get_ticks_usec() const;
    virtual String get_stdin_string(bool p_block = true);
	virtual int get_audio_driver_count() const;
	virtual const char *get_audio_driver_name(int p_driver) const;
    OS_Vita();
    
};

#endif
