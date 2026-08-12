import unreal

MAP_PATH = "/Game/Map/NewMap"
GAME_MODE_CLASS_PATH = "/Game/NumberBaseball/BP_NBGameMode.BP_NBGameMode_C"

if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    raise RuntimeError(f"Map not found: {MAP_PATH}")

game_mode_class = unreal.load_class(None, GAME_MODE_CLASS_PATH)
if not game_mode_class:
    raise RuntimeError(f"GameMode class not found: {GAME_MODE_CLASS_PATH}")

level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not level_subsystem.load_level(MAP_PATH):
    raise RuntimeError(f"Could not load map: {MAP_PATH}")

world = unreal.EditorLevelLibrary.get_editor_world()
world_settings = world.get_world_settings()
world_settings.set_editor_property("default_game_mode", game_mode_class)

if not level_subsystem.save_current_level():
    raise RuntimeError("Could not save the configured map")

unreal.log("[NumberBaseball] NewMap now explicitly uses BP_NBGameMode.")
