#include "block_hotloader.h"

void initalize_asset_manager()
{
	manager.size_assets = 64;
	manager.num_assets = 0;
	manager.assets = push_array(Asset, manager.size_assets);
	zero_array(manager.assets, Asset, manager.size_assets);

	// Register all the asset types
	register_asset_type(AFT_TEXTURE, load_texture, reload_texture, destroy_texture);
	register_asset_type(AFT_SHADER, compile_progam, recompile_program, destroy_program);
	register_asset_type(AFT_ATLAS, load_sprite_atlas, reload_sprite_atlas, destroy_sprite_atlas);
	register_asset_type(AFT_SOUND, load_sound, reload_sound, destroy_sound);
	register_asset_type(AFT_FONT, load_font, reload_font, destroy_font);
	register_asset_type(AFT_LEVEL, load_level, reload_level, destroy_level);

}

inline
AssetType asset_type(AssetFileType type_id)
{
	AssetType type = manager.asset_types[type_id];
	ASSERT(type.type == type_id);
	return type;
}

inline 
AssetType asset_type(Asset *asset)
{
	return asset_type(asset->type);
}

void destroy_asset_manager()
{
	for (u64 i = 0; i < manager.num_assets; i++)
	{ 
		Asset asset = manager.assets[i];
		if (asset.last_edit != 0)
		{
			AssetType type = asset_type(asset.type);
			type.destroy(manager.assets[i]);
			pop_memory((void *) manager.assets[i].path); // unallocate the paths
		}
	}
	pop_memory(manager.assets);
}

void register_asset_type(AssetFileType type, 
		Asset (*load)(const char *), 
		void (*reload)(Asset *), 
		void (*destroy)(Asset))
{
	ASSERT(load);
	ASSERT(reload);
	ASSERT(destroy);
	manager.asset_types[type] = {type, load, reload, destroy};
}

AssetID load_asset(AssetFileType file_type, const char *path)
{
	if (manager.num_assets == manager.size_assets)
	{
		// Scale up!
		manager.size_assets *= 2;
		manager.assets = resize_array(manager.assets, Asset, manager.size_assets);
		ASSERT(manager.assets);
	}

	AssetType type = asset_type(file_type);
	Asset asset = type.load(path);
	// This doesn't need to be done in the load function.
	asset.path = str_copy(path);
	asset.type = file_type;
	asset.last_edit = file_timestamp(path);
	asset.reload_timer = {-1.0f, 0.10f};


	AssetID id;
	id.pos = (s32) manager.num_assets++;
	manager.assets[id.pos] = asset;
	return id;
}

bool needs_reload(Asset *asset)
{
	u64 last_edit = file_timestamp(asset->path);
	if (last_edit != asset->last_edit)
	{
		asset->last_edit = last_edit;
		asset->reload_timer = Timer::reset(asset->reload_timer);
	}
	else if (check(asset->reload_timer))
	{
		asset->reload_timer = Timer::stop(asset->reload_timer);
		return true;
	}
	return false;
}

// Helper function to check if an asset requiers a reload
void check_assets_for_reload()
{
	for (u64 i = 0; i < manager.num_assets; i++)
	{
		Asset *asset = manager.assets + i;
		if (needs_reload(asset))
		{
			AssetType type = manager.asset_types[asset->type];
			// Maybe we should pass a pointer here.
			type.reload(asset);
		}
	}
}

Asset find_asset(AssetFileType type, const char *path)
{
	for (u64 i = 0; i < manager.num_assets; i++)
	{
		Asset *asset = manager.assets + i;
		if (asset->type != type) continue;
		if (str_eq(asset->path, path))
			return *asset;
	}
	// TODO: Should we load it here?
	return {};
}

Asset find_asset(u32 pos)
{
	ASSERT(pos < manager.num_assets);
	return manager.assets[pos];
}

Asset find_asset(AssetID id)
{
	Asset result = find_asset(id.pos);
	//ASSERT(result.id.uid == id.uid);
	return result;
}


