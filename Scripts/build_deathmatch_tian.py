from datetime import datetime

import unreal


LEVEL_PATH = "/Game/Maps/DM_Tian"
BACKUP_BASE_PATH = "/Game/Maps/DM_Tian_Backup_AutoBeforeRebuild"

CUBE_MESH_PATHS = [
    "/Engine/BasicShapes/Cube.Cube",
    "/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
]
SPHERE_MESH_PATHS = [
    "/Engine/BasicShapes/Sphere.Sphere",
]
BLACK_MATERIAL_PATH = "/Game/Materials/M_Env_Black.M_Env_Black"
GENERATED_PREFIX = "DM_Tian_"


def log(message):
    unreal.log(f"[DeathmatchTian] {message}")


def load_asset_checked(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def load_first_asset_checked(paths):
    for path in paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset is not None:
            log(f"Using asset: {path}")
            return asset
    raise RuntimeError(f"Missing all candidate assets: {paths}")


def make_unique_backup_path():
    if not unreal.EditorAssetLibrary.does_asset_exist(BACKUP_BASE_PATH):
        return BACKUP_BASE_PATH

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    return f"{BACKUP_BASE_PATH}_{timestamp}"


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


def spawn_static_box(label, location, scale, mesh, material):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.tags = [unreal.Name("EchoMapWall")]

    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    if material is not None:
        component.set_material(0, material)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_collision_profile_name("BlockAll")
    return actor


def spawn_marker_sphere(label, location, scale, mesh):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    return actor


def spawn_player_start(label, location, yaw):
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(*location),
        unreal.Rotator(0.0, yaw, 0.0),
    )
    start.set_actor_label(f"{GENERATED_PREFIX}{label}")
    return start


def configure_world_settings():
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        log("Skipped GameMode setup: editor world unavailable.")
        return

    world_settings = world.get_world_settings()
    deathmatch_class = unreal.load_class(None, "/Script/BallDarkEcho.EchoDeathmatchGameMode")
    if deathmatch_class is None:
        log("Skipped GameMode setup: /Script/BallDarkEcho.EchoDeathmatchGameMode not found. Compile C++ first.")
        return

    if set_actor_property_if_present(world_settings, "default_game_mode", deathmatch_class):
        log("Set World Settings default_game_mode to EchoDeathmatchGameMode.")


def build_tian_layout(cube_mesh, material):
    # BasicShape Cube is 100 uu. Scales below are dimensions / 100.
    half_size = 2000.0
    wall_thickness = 120.0
    wall_height = 320.0
    floor_z = -55.0
    wall_z = 155.0

    spawn_static_box("Floor", (0.0, 0.0, floor_z), (42.0, 42.0, 1.0), cube_mesh, material)

    # Outer square border.
    spawn_static_box("Outer_West", (-half_size, 0.0, wall_z), (wall_thickness / 100.0, 40.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Outer_East", (half_size, 0.0, wall_z), (wall_thickness / 100.0, 40.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Outer_South", (0.0, -half_size, wall_z), (40.0, wall_thickness / 100.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Outer_North", (0.0, half_size, wall_z), (40.0, wall_thickness / 100.0, wall_height / 100.0), cube_mesh, material)

    # Central cross. This makes the "田" shape. Small center gaps keep the first combat test traversable.
    gap = 420.0
    segment_half = (half_size - gap * 0.5) * 0.5
    segment_center = gap * 0.5 + segment_half

    spawn_static_box("Center_Vertical_North", (0.0, segment_center, wall_z), (wall_thickness / 100.0, segment_half * 2.0 / 100.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Center_Vertical_South", (0.0, -segment_center, wall_z), (wall_thickness / 100.0, segment_half * 2.0 / 100.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Center_Horizontal_East", (segment_center, 0.0, wall_z), (segment_half * 2.0 / 100.0, wall_thickness / 100.0, wall_height / 100.0), cube_mesh, material)
    spawn_static_box("Center_Horizontal_West", (-segment_center, 0.0, wall_z), (segment_half * 2.0 / 100.0, wall_thickness / 100.0, wall_height / 100.0), cube_mesh, material)

    # Low center reference block helps players read the four-room shape through echo.
    spawn_static_box("Center_Low_Block", (0.0, 0.0, 25.0), (2.4, 2.4, 0.6), cube_mesh, material)


def add_player_starts():
    starts = [
        ("PlayerStart_NW_A", (-1450.0, 1450.0, 120.0), -45.0),
        ("PlayerStart_NE_A", (1450.0, 1450.0, 120.0), -135.0),
        ("PlayerStart_SW_A", (-1450.0, -1450.0, 120.0), 45.0),
        ("PlayerStart_SE_A", (1450.0, -1450.0, 120.0), 135.0),
        ("PlayerStart_NW_B", (-650.0, 1450.0, 120.0), -75.0),
        ("PlayerStart_NE_B", (650.0, 1450.0, 120.0), -105.0),
        ("PlayerStart_SW_B", (-650.0, -1450.0, 120.0), 75.0),
        ("PlayerStart_SE_B", (650.0, -1450.0, 120.0), 105.0),
    ]

    for label, location, yaw in starts:
        spawn_player_start(label, location, yaw)


def add_pickup_markers(sphere_mesh):
    # Temporary non-gameplay markers for future Overcharge/Silent/Shield pickup placement.
    markers = [
        ("PickupMarker_North", (0.0, 1300.0, 140.0)),
        ("PickupMarker_South", (0.0, -1300.0, 140.0)),
        ("PickupMarker_East", (1300.0, 0.0, 140.0)),
        ("PickupMarker_West", (-1300.0, 0.0, 140.0)),
    ]
    for label, location in markers:
        spawn_marker_sphere(label, location, (0.7, 0.7, 0.7), sphere_mesh)


def add_orientation_lights():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 1200.0),
        unreal.Rotator(-65.0, 25.0, 0.0),
    )
    light.set_actor_label(f"{GENERATED_PREFIX}DimDirectionalLight")
    component = light.get_component_by_class(unreal.DirectionalLightComponent)
    if component:
        component.set_editor_property("intensity", 0.12)

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 900.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    sky.set_actor_label(f"{GENERATED_PREFIX}DimSkyLight")
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    if component:
        component.set_editor_property("intensity", 0.04)


def main():
    cube_mesh = load_first_asset_checked(CUBE_MESH_PATHS)
    sphere_mesh = load_first_asset_checked(SPHERE_MESH_PATHS)
    material = load_asset_checked(BLACK_MATERIAL_PATH)

    open_or_create_level()
    clear_level()
    configure_world_settings()
    build_tian_layout(cube_mesh, material)
    add_player_starts()
    add_pickup_markers(sphere_mesh)
    add_orientation_lights()

    unreal.EditorLevelLibrary.save_current_level()
    log("Saved DM_Tian deathmatch test map.")


if __name__ == "__main__":
    main()
