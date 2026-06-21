import unreal


LEVEL_PATH = "/Game/Maps/MenuLobby"
BACKUP_BASE_PATH = "/Game/Maps/MenuLobby_Backup_AutoBeforeRebuild"
GENERATED_PREFIX = "MenuLobby_"


def log(message):
    unreal.log(f"[MenuLobby] {message}")


def make_unique_backup_path():
    if not unreal.EditorAssetLibrary.does_asset_exist(BACKUP_BASE_PATH):
        return BACKUP_BASE_PATH

    suffix = 1
    while unreal.EditorAssetLibrary.does_asset_exist(f"{BACKUP_BASE_PATH}_{suffix}"):
        suffix += 1
    return f"{BACKUP_BASE_PATH}_{suffix}"


def open_or_create_level():
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        backup_path = make_unique_backup_path()
        if not unreal.EditorAssetLibrary.duplicate_asset(LEVEL_PATH, backup_path):
            raise RuntimeError(f"Failed to create level backup: {backup_path}")

        unreal.EditorAssetLibrary.save_asset(backup_path, only_if_is_dirty=False)
        log(f"Backed up {LEVEL_PATH} to {backup_path}")
        unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
        return

    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError(f"Failed to create level: {LEVEL_PATH}")

    log(f"Created new level: {LEVEL_PATH}")


def is_keep_actor(actor):
    keep_types = (
        unreal.WorldSettings,
        unreal.Brush,
        unreal.LevelScriptActor,
    )
    return isinstance(actor, keep_types)


def clear_level():
    actors = list(unreal.EditorLevelLibrary.get_all_level_actors())
    deleted = 0
    for actor in actors:
        if actor is None or is_keep_actor(actor):
            continue
        unreal.EditorLevelLibrary.destroy_actor(actor)
        deleted += 1
    log(f"Deleted {deleted} actors from {LEVEL_PATH}")


def set_actor_property_if_present(actor, property_name, value):
    try:
        actor.set_editor_property(property_name, value)
        return True
    except Exception as exc:
        object_name = actor.get_name() if hasattr(actor, "get_name") else str(actor)
        log(f"Skipped property {property_name} on {object_name}: {exc}")
        return False


def configure_world_settings():
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        raise RuntimeError("Editor world unavailable.")

    world_settings = world.get_world_settings()
    lobby_class = unreal.load_class(None, "/Script/BallDarkEcho.EchoLobbyGameMode")
    if lobby_class is None:
        raise RuntimeError("Missing /Script/BallDarkEcho.EchoLobbyGameMode. Compile C++ first.")

    if set_actor_property_if_present(world_settings, "default_game_mode", lobby_class):
        log("Set World Settings default_game_mode to EchoLobbyGameMode.")


def spawn_player_start():
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0.0, 0.0, 120.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    start.set_actor_label(f"{GENERATED_PREFIX}PlayerStart")


def spawn_lighting():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(-300.0, -300.0, 500.0),
        unreal.Rotator(-45.0, -35.0, 0.0),
    )
    light.set_actor_label(f"{GENERATED_PREFIX}DirectionalLight")
    light.light_component.set_editor_property("intensity", 2.0)

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 300.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    sky.set_actor_label(f"{GENERATED_PREFIX}SkyLight")
    sky.light_component.set_editor_property("intensity", 0.35)


def main():
    open_or_create_level()
    clear_level()
    configure_world_settings()
    spawn_player_start()
    spawn_lighting()
    unreal.EditorLevelLibrary.save_current_level()
    log(f"Saved {LEVEL_PATH}")


main()
