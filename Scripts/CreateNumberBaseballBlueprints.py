import unreal

ROOT = "/Game/NumberBaseball"


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_blueprint(name, parent_class):
    asset_path = f"{ROOT}/{name}"
    existing = (unreal.EditorAssetLibrary.load_asset(asset_path)
                if unreal.EditorAssetLibrary.does_asset_exist(asset_path) else None)
    if existing:
        unreal.log(f"[NumberBaseball] Reusing {asset_path}")
        return existing

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, ROOT, unreal.Blueprint, factory
    )
    if not asset:
        raise RuntimeError(f"Failed to create {asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log(f"[NumberBaseball] Created {asset_path}")
    return asset


def generated_class(blueprint):
    return unreal.load_class(None, blueprint.get_path_name() + "_C")


ensure_directory(ROOT)

player_state_bp = create_blueprint("BP_NBPlayerState", unreal.NBPlayerState)
game_state_bp = create_blueprint("BP_NBGameState", unreal.NBGameState)
controller_bp = create_blueprint("BP_NBPlayerController", unreal.NBPlayerController)
widget_bp = create_blueprint("WBP_NBChat", unreal.NBChatWidget)
game_mode_bp = create_blueprint("BP_NBGameMode", unreal.NBGameMode)

# Make the Blueprint GameMode use the other Blueprint subclasses. The gameplay
# remains server-authoritative in C++, while designers can override defaults and
# add Blueprint presentation logic without changing source code.
game_mode_cdo = unreal.get_default_object(generated_class(game_mode_bp))
game_mode_cdo.set_editor_property("player_state_class", generated_class(player_state_bp))
game_mode_cdo.set_editor_property("game_state_class", generated_class(game_state_bp))
game_mode_cdo.set_editor_property("player_controller_class", generated_class(controller_bp))
controller_cdo = unreal.get_default_object(generated_class(controller_bp))
controller_cdo.set_editor_property("chat_widget_class", generated_class(widget_bp))

for asset in (player_state_bp, game_state_bp, controller_bp, widget_bp, game_mode_bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(asset)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)

unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
unreal.log("[NumberBaseball] Blueprint asset creation completed successfully.")
