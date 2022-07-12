/*************************************************************************/
/*  export.cpp                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "export.h"
#include "zipper.h"

#define TEMPLATE_RELEASE "eboot.bin"

class ExportPluginVita : public EditorExportPlugin {
public:
	Vector<uint8_t> editor_id_vec;

protected:
	virtual void _export_begin(const Set<String> &p_features, bool p_debug, const String &p_path, int p_flags) {
		if (editor_id_vec.size() != 0) {
			add_file("custom_editor_id", editor_id_vec, false);
		}
	}
};

class EditorExportPlatformVita : public EditorExportPlatform {

	GDCLASS(EditorExportPlatformVita, EditorExportPlatform)

	Ref<ImageTexture> logo;

    ExportPluginVita *export_plugin;

public:

	virtual void get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) {
        String driver = ProjectSettings::get_singleton()->get("rendering/quality/driver/driver_name");
        if (driver == "GLES2") {
            r_features->push_back("etc");
        } else if (driver == "GLES3") {
            if (ProjectSettings::get_singleton()->get("rendering/quality/driver/fallback_to_gles2")) {
                r_features->push_back("etc");
            }
        }
	}

	virtual void get_export_options(List<ExportOption> *r_options) {
        String title = ProjectSettings::get_singleton()->get("application/config/name");
		r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "application/fused_build"), false));
		r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/custom_editor_id"), ""));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/title_short", PROPERTY_HINT_PLACEHOLDER_TEXT, title), title));
		r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/title_long", PROPERTY_HINT_PLACEHOLDER_TEXT, title), title));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/title_id", PROPERTY_HINT_PLACEHOLDER_TEXT, "GDOT00001 (Make sure it's CAPITALIZED and 9 characters MAX"), "GDOT00001"));
		r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/author", PROPERTY_HINT_PLACEHOLDER_TEXT, "Game Author"), "Stary & Cpasjuste"));
		r_options->push_back(ExportOption(PropertyInfo(Variant::INT, "application/parental_level"), 1));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/version", PROPERTY_HINT_PLACEHOLDER_TEXT, "Game Version XX.YY"), "01.00"));
		r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/bubble_icon_128x128", PROPERTY_HINT_GLOBAL_FILE, "*.png"), ""));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/launch_splash_960x544", PROPERTY_HINT_GLOBAL_FILE, "*.png"), ""));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/livearea_bg_840x500", PROPERTY_HINT_GLOBAL_FILE, "*.png"), ""));
        r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/livearea_startup_button_280x158", PROPERTY_HINT_GLOBAL_FILE, "*.png"), ""));
    }

	virtual String get_name() const {
		return "Vita";
	}

	virtual String get_os_name() const {
		return "Vita";
	}

	virtual Ref<Texture> get_logo() const {
		return logo;
	}

	virtual Ref<Texture> get_run_icon() const {
		return logo;
	}

	virtual bool poll_devices() {
		return false;
	}

	virtual int get_device_count() const {
		return 2;
	}

	virtual String get_device_name(int p_device) const {
		if(p_device) {
			return "smdc:";
		} else {
			return "romfs:";
		}
	}

	virtual String get_device_info(int p_device) const {
		if(p_device) {
			return "smdc:";
		} else {
			return "romfs:";
		}
	}

	virtual Error run(const Ref<EditorExportPreset> &p_preset, int p_device, int p_debug_flags) {
		return OK;
	}

	virtual bool can_export(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates) const {
		String err;
		r_missing_templates =
				find_export_template(TEMPLATE_RELEASE) == String();

		bool valid = !r_missing_templates;
		String etc_error = test_etc2();
		if (etc_error != String()) {
			err += etc_error;
			valid = false;
		}

		if (!err.empty()) {
			r_error = err;
		}

		return valid;
	}

	virtual List<String> get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const {
		List<String> list;
		list.push_back("vpk");
		return list;
	}

    void makeSFO(ParamSFOStruct *sfo) {

        String exe_ext;
		if (OS::get_singleton()->get_name() == "Windows") {
			exe_ext = ".exe";
		}
		String mksfoex = OS::get_singleton()->get_executable_path().get_base_dir() + "/vita-mksfoex" + exe_ext;

        if (FileAccess::exists(mksfoex)) {
			List<String> args;
			int ec;

            args.push_back("-d");
            args.push_back("ATTRIBUTE2=12"); // Always want to set extended memory mode
			args.push_back("-s");
			args.push_back("TITLE_ID="+sfo->title);
			args.push_back("-s");
			args.push_back("APP_VER="+sfo->version);
			args.push_back("\""+sfo->title+"\"");
			args.push_back("param.sfo");

			OS::get_singleton()->execute(mksfoex, args, true, NULL, NULL, &ec);
		} else {
			EditorNode::get_singleton()->show_warning(TTR("vita-mksfoex not found!"));
		}
    }

    void create_vpk(String outVpk, String dir) {
        zipFile zip;
        zip = zipOpen(outVpk.utf8().ptr(), 0);
        if (!zip) {
            EditorNode::get_singleton()->show_warning(TTR("Could not open zip file"));
            zipClose(zip, NULL); 
            return;
        }
        if (!zipper_add_dir(zip, dir.utf8().ptr())) {
            EditorNode::get_singleton()->show_warning(TTR("failed to write dir"));
            zipClose(zip, NULL); 
            return;
        }
        zipClose(zip, NULL); 
    }

	virtual Error export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0) {
		ExportNotifier notifier(*this, p_preset, p_debug, p_path, p_flags);

        const String base_dir = p_path.get_base_dir();
        const String base_path = p_path.get_basename();
        const String base_name = p_path.get_file().get_basename();

        if (!DirAccess::exists(base_dir)) {
            return ERR_FILE_BAD_PATH;
        }

        String eboot_path = find_export_template(TEMPLATE_RELEASE);
        if (eboot_path != String() && !FileAccess::exists(eboot_path)) {
            add_message(EXPORT_MESSAGE_ERROR, TTR("Prepare Templates"), vformat(TTR("Template file not found: \"%s\"."), eboot_path));
            return ERR_FILE_NOT_FOUND;
        }

        DirAccess *da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

        Error err;
		// update nro icon/title/author/version
        ParamSFOStruct *sfo = memnew(ParamSFOStruct);

		sfo->title = p_preset->get("application/title_short");
		sfo->title_long = p_preset->get("application/title_long");
        sfo->title_id = p_preset->get("application/title_id");
        sfo->author = p_preset->get("application/author");
		sfo->version = p_preset->get("application/version");
        sfo->parental_level = p_preset->get("application/parental_level");

        String icon = p_preset->get("application/bubble_icon_128x128");
        String splash = p_preset->get("application/launch_splash_960x544");
        String livearea_bg = p_preset->get("application/livearea_bg_840x500");
        String livearea_startup_button = p_preset->get("application/livearea_startup_button_280x158");

        String cache = EditorSettings::get_singleton()->get_cache_dir();
        String app_dir = cache.plus_file("app");
        da->make_dir(app_dir);
        String game_data_dir = app_dir.plus_file("game_data");
        da->make_dir(game_data_dir);
        da->make_dir_recursive(app_dir.plus_file("sce_sys/livearea/contents"));

        err = save_pack(p_preset, game_data_dir.plus_file("game.pck"));
        makeSFO(sfo);
        if (err == OK) {
            if (FileAccess::exists(eboot_path)) {
                da->copy(eboot_path, app_dir.plus_file("eboot.bin"));
            }
            if (icon != String() && FileAccess::exists(icon)) {
                da->copy(icon, app_dir.plus_file("sce_sys/icon0.png"));
            }
            if (splash != String() && FileAccess::exists(splash)) {
                da->copy(splash, app_dir.plus_file("sce_sys/pic0.png"));
            }
            if (livearea_bg != String() && FileAccess::exists(livearea_bg)) {
                da->copy(livearea_bg, app_dir.plus_file("sce_sys/livearea/contents/bg.png"));
            }
            if (livearea_startup_button != String() && FileAccess::exists(livearea_startup_button)) {
                da->copy(livearea_startup_button, app_dir.plus_file("sce_sys/livearea/contents/startup.png"));
            }
            if (FileAccess::exists("template.xml")) {
                da->copy("template.xml", app_dir.plus_file("sce_sys/livearea/contents/template.xml"));
            }
        }

        create_vpk(sfo->title + ".vpk", app_dir);
        memdelete(sfo);
        memdelete(da);
        da = nullptr;

        return OK;
	}

	virtual void get_platform_features(List<String> *r_features) {
        r_features->push_back("mobile");
        r_features->push_back(get_os_name());
	}

	virtual void resolve_platform_feature_priorities(const Ref<EditorExportPreset> &p_preset, Set<String> &p_features) {
	}

	EditorExportPlatformVita() {

		Ref<Image> img = memnew(Image(_vita_logo));
		logo.instance();
		logo->create_from_image(img);

        export_plugin = memnew(ExportPluginVita);
		EditorExport::get_singleton()->add_export_plugin(export_plugin);
	}

	~EditorExportPlatformVita() {
	}
};

void register_vita_exporter() {

	Ref<EditorExportPlatformVita> exporter = Ref<EditorExportPlatformVita>(memnew(EditorExportPlatformVita));
	EditorExport::get_singleton()->add_export_platform(exporter);
}