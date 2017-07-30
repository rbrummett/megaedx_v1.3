#include "libretro.h"
#include <windows.h>

static HMODULE h;

static void (*pretro_set_environment)(retro_environment_t);
void retro_set_environment(retro_environment_t p)
{
	pretro_set_environment(p);
}

static void (*pretro_set_video_refresh)(retro_video_refresh_t);
void retro_set_video_refresh(retro_video_refresh_t p)
{
	pretro_set_video_refresh(p);
}

static void (*pretro_set_audio_sample)(retro_audio_sample_t);
void retro_set_audio_sample(retro_audio_sample_t p)
{
	pretro_set_audio_sample(p);
}

static void (*pretro_set_audio_sample_batch)(retro_audio_sample_batch_t);
void retro_set_audio_sample_batch(retro_audio_sample_batch_t p)
{
	pretro_set_audio_sample_batch(p);
}

static void (*pretro_set_input_poll)(retro_input_poll_t);
void retro_set_input_poll(retro_input_poll_t p)
{
	pretro_set_input_poll(p);
}

static void (*pretro_set_input_state)(retro_input_state_t);
void retro_set_input_state(retro_input_state_t p)
{
	pretro_set_input_state(p);
}

//static void(*pretro_set_frame_time_callback)(retro_frame_time_callback_t);
//void retro_set_frame_time_callback(retro_frame_time_callback_t p)
//{
//	pretro_set_frame_time_callback(p);
//}

static void (*pretro_init)(void);
void retro_init(void)
{
	pretro_init();
}

static void (*pretro_deinit)(void);
void retro_deinit(void)
{
	pretro_deinit();
	FreeLibrary(h);
}

static unsigned (*pretro_api_version)(void);
unsigned retro_api_version(void)
{
	return pretro_api_version();
}

static void (*pretro_get_system_info)(struct retro_system_info *info);
void retro_get_system_info(struct retro_system_info *info)
{
	pretro_get_system_info(info);
}

static void (*pretro_get_system_av_info)(struct retro_system_av_info *info);
void retro_get_system_av_info(struct retro_system_av_info *info)
{
	pretro_get_system_av_info(info);
}

static void (*pretro_set_controller_port_device)(unsigned port, unsigned device);
void retro_set_controller_port_device(unsigned port, unsigned device)
{
	pretro_set_controller_port_device(port, device);
}

static void (*pretro_reset)(void);
void retro_reset(void)
{
	pretro_reset();
}

static void (*pretro_run)(void);
void retro_run(void)
{
	pretro_run();
}

static size_t (*pretro_serialize_size)(void);
size_t retro_serialize_size(void)
{
	return pretro_serialize_size();
}

static bool (*pretro_serialize)(void *data, size_t size);
bool retro_serialize(void *data, size_t size)
{
	return pretro_serialize(data, size);
}

static bool (*pretro_unserialize)(const void *data, size_t size);
bool retro_unserialize(const void *data, size_t size)
{
	return pretro_unserialize(data, size);
}

static void (*pretro_cheat_reset)(void);
void retro_cheat_reset(void)
{
	pretro_cheat_reset();
}

static void (*pretro_cheat_set)(unsigned index, bool enabled, const char *code);
void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
	pretro_cheat_set(index, enabled, code);
}

static bool (*pretro_load_game)(const struct retro_game_info *game);
bool retro_load_game(const struct retro_game_info *game)
{
	return pretro_load_game(game);
}

static bool (*pretro_load_game_special)(unsigned game_type, const struct retro_game_info *info, size_t num_info);
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
	return pretro_load_game_special(game_type, info, num_info);
}

static void (*pretro_unload_game)(void);
void retro_unload_game(void)
{
	pretro_unload_game();
}

static unsigned (*pretro_get_region)(void);
unsigned retro_get_region(void)
{
	return pretro_get_region();
}

static void* (*pretro_get_memory_data)(unsigned id);
void* retro_get_memory_data(unsigned id)
{
	return pretro_get_memory_data(id);
}

static size_t (*pretro_get_memory_size)(unsigned id);
size_t retro_get_memory_size(unsigned id)
{
	return pretro_get_memory_size(id);
}

bool retro_load(LPCWSTR dllName)
{
	h=LoadLibraryW(dllName);
	if (!h) return false;
#define die() do { FreeLibrary(h); return false; } while(0)
#define libload(name) GetProcAddress(h, name)
#ifdef __GNUC__
#define load(name) if (!(p##name=(__typeof(p##name))libload(#name))) die()//shut up about that damn type punning
#else
#define load(name) if (!(*(void**)(&p##name)=(void*)libload(#name))) die()
#endif
	load(retro_set_environment);
	load(retro_set_video_refresh);
	load(retro_set_audio_sample);
	load(retro_set_audio_sample_batch);
	load(retro_set_input_poll);
	load(retro_set_input_state);
	load(retro_init);
	load(retro_deinit);
	load(retro_api_version);
	load(retro_get_system_info);
	load(retro_get_system_av_info);
	load(retro_set_controller_port_device);
	load(retro_reset);
	load(retro_run);
	load(retro_serialize_size);
	load(retro_serialize);
	load(retro_unserialize);
	load(retro_cheat_reset);
	load(retro_cheat_set);
	load(retro_load_game);
	load(retro_load_game_special);
	load(retro_unload_game);
	load(retro_get_region);
	load(retro_get_memory_data);
	load(retro_get_memory_size);
	
	if (retro_api_version()!=RETRO_API_VERSION) die();
	
	return true;
}
