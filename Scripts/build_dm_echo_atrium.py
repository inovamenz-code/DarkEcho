from datetime import datetime
import math

import unreal


LEVEL_PATH = "/Game/Maps/DM_EchoAtrium"
BACKUP_BASE_PATH = "/Game/Maps/DM_EchoAtrium_Backup_AutoBeforeRebuild"

CUBE_MESH_PATHS = [
    "/Engine/BasicShapes/Cube.Cube",
    "/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
]
SPHERE_MESH_PATHS = [
    "/Engine/BasicShapes/Sphere.Sphere",
]
ECHO_WAVE_MATERIAL_PATH = "/Game/Materials/M_Env_EchoWave_Test.M_Env_EchoWave_Test"
DEATHMATCH_GAME_MODE_BLUEPRINT_PATH = "/Game/BluePrints/BP_EchoDeathmatchGameMode.BP_EchoDeathmatchGameMode"
GENERATED_PREFIX = "DM_EchoAtrium_"


def log(message):
    unreal.log(f"[DeathmatchEchoAtrium] {message}")


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


def load_blueprint_generated_class(path):
    blueprint = unreal.EditorAssetLibrary.load_asset(path)
    if blueprint is None:
        return None

    generated_class = blueprint.generated_class()
    if generated_class is None:
        log(f"Blueprint has no generated class: {path}")
    return generated_class


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


def spawn_static_box(label, location, scale, mesh, material, rotation=(0.0, 0.0, 0.0), tag="EchoMapWall"):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(*rotation),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    if tag:
        actor.tags = [unreal.Name(tag)]

    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    if material is not None:
        component.set_material(0, material)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_collision_profile_name("BlockAll")
    return actor


def spawn_marker_sphere(label, location, scale, mesh, tag):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.tags = [unreal.Name(tag)]
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
    deathmatch_class = load_blueprint_generated_class(DEATHMATCH_GAME_MODE_BLUEPRINT_PATH)
    if deathmatch_class is not None:
        if set_actor_property_if_present(world_settings, "default_game_mode", deathmatch_class):
            log(f"Set World Settings default_game_mode to {DEATHMATCH_GAME_MODE_BLUEPRINT_PATH}.")
        return

    deathmatch_class = unreal.load_class(None, "/Script/BallDarkEcho.EchoDeathmatchGameMode")
    if deathmatch_class is None:
        log("Skipped GameMode setup: no Blueprint or C++ EchoDeathmatchGameMode found.")
        return

    if set_actor_property_if_present(world_settings, "default_game_mode", deathmatch_class):
        log("Set World Settings default_game_mode to C++ EchoDeathmatchGameMode fallback.")


def spawn_floor_plate(label, center, size_x, size_y, mesh, material):
    spawn_static_box(
        label,
        (center[0], center[1], center[2] - 55.0),
        (size_x / 100.0, size_y / 100.0, 1.0),
        mesh,
        material,
        tag="EchoMapFloor",
    )


def spawn_wall(label, x, y, z, size_x, size_y, height, mesh, material):
    spawn_static_box(
        label,
        (x, y, z + height * 0.5),
        (size_x / 100.0, size_y / 100.0, height / 100.0),
        mesh,
        material,
    )


def spawn_low_cover(label, x, y, z, size_x, size_y, height, mesh, material):
    spawn_static_box(
        label,
        (x, y, z + height * 0.5),
        (size_x / 100.0, size_y / 100.0, height / 100.0),
        mesh,
        material,
        tag="EchoMapCover",
    )


def spawn_pillar(label, x, y, z, width, height, mesh, material):
    spawn_static_box(
        label,
        (x, y, z + height * 0.5),
        (width / 100.0, width / 100.0, height / 100.0),
        mesh,
        material,
    )


def build_floor_layer(layer_name, z, half_extent, atrium_half, mesh, material, b_fill_atrium=False):
    # Floor plates are tiled so upper layers can leave real corner stairwell holes.
    stairwell_inner = 1450.0
    edges = [-half_extent, -stairwell_inner, -atrium_half, atrium_half, stairwell_inner, half_extent]
    b_has_corner_stairwells = not b_fill_atrium

    for x_index in range(len(edges) - 1):
        x_min = edges[x_index]
        x_max = edges[x_index + 1]
        for y_index in range(len(edges) - 1):
            y_min = edges[y_index]
            y_max = edges[y_index + 1]

            b_center_atrium = not b_fill_atrium and x_min >= -atrium_half and x_max <= atrium_half and y_min >= -atrium_half and y_max <= atrium_half
            b_corner_stairwell = (
                b_has_corner_stairwells
                and (x_max <= -stairwell_inner or x_min >= stairwell_inner)
                and (y_max <= -stairwell_inner or y_min >= stairwell_inner)
            )
            if b_center_atrium or b_corner_stairwell:
                continue

            center_x = (x_min + x_max) * 0.5
            center_y = (y_min + y_max) * 0.5
            size_x = x_max - x_min
            size_y = y_max - y_min
            spawn_floor_plate(f"{layer_name}_Floor_Tile_{x_index}_{y_index}", (center_x, center_y, z), size_x, size_y, mesh, material)


def build_outer_shell(layer_name, z, half_extent, mesh, material):
    wall_h = 360.0
    thick = 140.0
    span = half_extent * 2.0
    spawn_wall(f"{layer_name}_Outer_West", -half_extent, 0.0, z, thick, span, wall_h, mesh, material)
    spawn_wall(f"{layer_name}_Outer_East", half_extent, 0.0, z, thick, span, wall_h, mesh, material)
    spawn_wall(f"{layer_name}_Outer_South", 0.0, -half_extent, z, span, thick, wall_h, mesh, material)
    spawn_wall(f"{layer_name}_Outer_North", 0.0, half_extent, z, span, thick, wall_h, mesh, material)


def build_atrium_rails(layer_name, z, atrium_half, mesh, material):
    rail_h = 120.0
    thick = 90.0
    span = atrium_half * 2.0
    gap = 720.0
    segment = (span - gap) * 0.5
    offset = gap * 0.5 + segment * 0.5

    spawn_low_cover(f"{layer_name}_AtriumRail_North_W", -offset, atrium_half, z, segment, thick, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_North_E", offset, atrium_half, z, segment, thick, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_South_W", -offset, -atrium_half, z, segment, thick, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_South_E", offset, -atrium_half, z, segment, thick, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_West_N", -atrium_half, offset, z, thick, segment, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_West_S", -atrium_half, -offset, z, thick, segment, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_East_N", atrium_half, offset, z, thick, segment, rail_h, mesh, material)
    spawn_low_cover(f"{layer_name}_AtriumRail_East_S", atrium_half, -offset, z, thick, segment, rail_h, mesh, material)


def build_tactical_rooms(layer_name, z, mesh, material):
    wall_h = 310.0
    thick = 110.0
    # Staggered partitions avoid clean sightlines while keeping at least two exits from major spaces.
    walls = [
        ("MidWest_Baffle_A", -1380.0, 420.0, thick, 960.0),
        ("MidWest_Baffle_B", -1760.0, -430.0, 760.0, thick),
        ("MidEast_Baffle_A", 1380.0, -420.0, thick, 960.0),
        ("MidEast_Baffle_B", 1760.0, 430.0, 760.0, thick),
        ("North_OffsetGate", -360.0, 1500.0, 980.0, thick),
        ("South_OffsetGate", 360.0, -1500.0, 980.0, thick),
    ]
    for suffix, x, y, sx, sy in walls:
        spawn_wall(f"{layer_name}_{suffix}", x, y, z, sx, sy, wall_h, mesh, material)

    covers = [
        ("AmbushCover_NW", -980.0, 1080.0, 520.0, 120.0),
        ("AmbushCover_NE", 980.0, 1080.0, 520.0, 120.0),
        ("AmbushCover_SW", -980.0, -1080.0, 520.0, 120.0),
        ("AmbushCover_SE", 980.0, -1080.0, 520.0, 120.0),
        ("PickupCover_N", 0.0, 1220.0, 640.0, 100.0),
        ("PickupCover_S", 0.0, -1220.0, 640.0, 100.0),
    ]
    for suffix, x, y, sx, sy in covers:
        spawn_low_cover(f"{layer_name}_{suffix}", x, y, z, sx, sy, 130.0, mesh, material)


def build_atrium_features(mesh, material):
    for z in (0.0, 450.0, 900.0):
        for label, x, y in [
            ("NW", -520.0, 520.0),
            ("NE", 520.0, 520.0),
            ("SW", -520.0, -520.0),
            ("SE", 520.0, -520.0),
        ]:
            spawn_pillar(f"Layer{int(z)}_AtriumPillar_{label}", x, y, z, 140.0, 360.0, mesh, material)

    # Broken central cover gives the ground floor a readable landmark without closing the drop shaft.
    spawn_low_cover("Ground_AtriumBrokenCover_North", 0.0, 330.0, 0.0, 520.0, 90.0, 115.0, mesh, material)
    spawn_low_cover("Ground_AtriumBrokenCover_East", 330.0, 0.0, 0.0, 90.0, 520.0, 115.0, mesh, material)
    spawn_low_cover("Ground_AtriumBrokenCover_South", -140.0, -360.0, 0.0, 420.0, 90.0, 115.0, mesh, material)


def spawn_ramp(label, start, end, width, thickness, mesh, material):
    sx, sy, sz = start
    ex, ey, ez = end
    dx = ex - sx
    dy = ey - sy
    dz = ez - sz
    horizontal = math.sqrt(dx * dx + dy * dy)
    length = math.sqrt(horizontal * horizontal + dz * dz)
    yaw = math.degrees(math.atan2(dy, dx))
    pitch = math.degrees(math.atan2(dz, horizontal))
    center = ((sx + ex) * 0.5, (sy + ey) * 0.5, (sz + ez) * 0.5)

    spawn_static_box(
        label,
        center,
        (length / 100.0, width / 100.0, thickness / 100.0),
        mesh,
        material,
        (pitch, yaw, 0.0),
        tag="EchoMapFloor",
    )

    rail_offset_x = -math.sin(math.radians(yaw)) * (width * 0.5 + 45.0)
    rail_offset_y = math.cos(math.radians(yaw)) * (width * 0.5 + 45.0)
    rail_scale = (length / 100.0, 0.55, 1.35)
    rail_z_offset = 95.0
    spawn_static_box(
        f"{label}_Rail_Left",
        (center[0] + rail_offset_x, center[1] + rail_offset_y, center[2] + rail_z_offset),
        rail_scale,
        mesh,
        material,
        (pitch, yaw, 0.0),
    )
    spawn_static_box(
        f"{label}_Rail_Right",
        (center[0] - rail_offset_x, center[1] - rail_offset_y, center[2] + rail_z_offset),
        rail_scale,
        mesh,
        material,
        (pitch, yaw, 0.0),
    )


def build_vertical_routes(mesh, material):
    # Corner ramps keep the atrium as combat space and make vertical routing easy to discover.
    corner_ramps = [
        ("NW", (-200.0, 2260.0, 0.0), (-2260.0, 2260.0, 450.0), (-2260.0, 200.0, 450.0), (-2260.0, 2260.0, 900.0)),
        ("NE", (200.0, 2260.0, 0.0), (2260.0, 2260.0, 450.0), (2260.0, 200.0, 450.0), (2260.0, 2260.0, 900.0)),
        ("SW", (-200.0, -2260.0, 0.0), (-2260.0, -2260.0, 450.0), (-2260.0, -200.0, 450.0), (-2260.0, -2260.0, 900.0)),
        ("SE", (200.0, -2260.0, 0.0), (2260.0, -2260.0, 450.0), (2260.0, -200.0, 450.0), (2260.0, -2260.0, 900.0)),
    ]

    for corner, l0_start, l1_entry, l1_start, l2_entry in corner_ramps:
        spawn_ramp(f"CornerRamp_{corner}_L0_To_L1", l0_start, l1_entry, 520.0, 65.0, mesh, material)
        spawn_ramp(f"CornerRamp_{corner}_L1_To_L2", l1_start, l2_entry, 500.0, 60.0, mesh, material)

    landings = [
        ("Landing_NW_L0", -200.0, 2260.0, 0.0, 8.0, 6.0),
        ("Landing_NW_L1", -2260.0, 2260.0, 450.0, 16.0, 16.0),
        ("Landing_NW_L1_RampStart", -2260.0, 200.0, 450.0, 8.0, 6.0),
        ("Landing_NW_L2", -2260.0, 2260.0, 900.0, 16.0, 16.0),
        ("Landing_NE_L0", 200.0, 2260.0, 0.0, 8.0, 6.0),
        ("Landing_NE_L1", 2260.0, 2260.0, 450.0, 16.0, 16.0),
        ("Landing_NE_L1_RampStart", 2260.0, 200.0, 450.0, 8.0, 6.0),
        ("Landing_NE_L2", 2260.0, 2260.0, 900.0, 16.0, 16.0),
        ("Landing_SW_L0", -200.0, -2260.0, 0.0, 8.0, 6.0),
        ("Landing_SW_L1", -2260.0, -2260.0, 450.0, 16.0, 16.0),
        ("Landing_SW_L1_RampStart", -2260.0, -200.0, 450.0, 8.0, 6.0),
        ("Landing_SW_L2", -2260.0, -2260.0, 900.0, 16.0, 16.0),
        ("Landing_SE_L0", 200.0, -2260.0, 0.0, 8.0, 6.0),
        ("Landing_SE_L1", 2260.0, -2260.0, 450.0, 16.0, 16.0),
        ("Landing_SE_L1_RampStart", 2260.0, -200.0, 450.0, 8.0, 6.0),
        ("Landing_SE_L2", 2260.0, -2260.0, 900.0, 16.0, 16.0),
    ]
    for suffix, x, y, z, sx, sy in landings:
        spawn_static_box(suffix, (x, y, z - 35.0), (sx, sy, 0.7), mesh, material, tag="EchoMapFloor")


def build_echo_atrium_layout(cube_mesh, material):
    half_extent = 2600.0
    atrium_half = 720.0
    for index, z in enumerate((0.0, 450.0, 900.0), start=1):
        layer_name = f"L{index}"
        build_floor_layer(layer_name, z, half_extent, atrium_half, cube_mesh, material, b_fill_atrium=(index == 1))
        build_outer_shell(layer_name, z, half_extent, cube_mesh, material)
        build_atrium_rails(layer_name, z, atrium_half, cube_mesh, material)
        build_tactical_rooms(layer_name, z, cube_mesh, material)

    build_atrium_features(cube_mesh, material)
    build_vertical_routes(cube_mesh, material)


def add_player_starts():
    starts = [
        ("PlayerStart_L0_NW", (-2220.0, 2220.0, 120.0), -45.0),
        ("PlayerStart_L0_NE", (2220.0, 2220.0, 120.0), -135.0),
        ("PlayerStart_L0_SW", (-2220.0, -2220.0, 120.0), 45.0),
        ("PlayerStart_L0_SE", (2220.0, -2220.0, 120.0), 135.0),
        ("PlayerStart_L1_N", (0.0, 2240.0, 570.0), -90.0),
        ("PlayerStart_L1_S", (0.0, -2240.0, 570.0), 90.0),
        ("PlayerStart_L2_W", (-2220.0, 0.0, 1020.0), 0.0),
        ("PlayerStart_L2_E", (2220.0, 0.0, 1020.0), 180.0),
    ]
    for label, location, yaw in starts:
        spawn_player_start(label, location, yaw)


def add_pickup_markers(sphere_mesh):
    markers = [
        ("PickupMarker_Overcharge_CentralWell", (0.0, 0.0, 1040.0), (0.9, 0.9, 0.9), "EchoPickup_Overcharge"),
        ("PickupMarker_Shield_AtriumLower", (0.0, 520.0, 140.0), (0.75, 0.75, 0.75), "EchoPickup_Shield"),
        ("PickupMarker_Shield_AtriumMiddle", (0.0, -520.0, 590.0), (0.75, 0.75, 0.75), "EchoPickup_Shield"),
        ("PickupMarker_Silent_NorthSideRoute", (-1360.0, 2220.0, 590.0), (0.7, 0.7, 0.7), "EchoPickup_Silent"),
        ("PickupMarker_Silent_SouthSideRoute", (1360.0, -2220.0, 1040.0), (0.7, 0.7, 0.7), "EchoPickup_Silent"),
        ("PickupMarker_OuterWest_Reset", (-2260.0, -360.0, 140.0), (0.65, 0.65, 0.65), "EchoPickup_Minor"),
        ("PickupMarker_OuterEast_Reset", (2260.0, 360.0, 590.0), (0.65, 0.65, 0.65), "EchoPickup_Minor"),
    ]
    for label, location, scale, tag in markers:
        spawn_marker_sphere(label, location, scale, sphere_mesh, tag)


def add_orientation_lights():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 2200.0),
        unreal.Rotator(-70.0, 35.0, 0.0),
    )
    light.set_actor_label(f"{GENERATED_PREFIX}DimDirectionalLight")
    component = light.get_component_by_class(unreal.DirectionalLightComponent)
    if component:
        component.set_editor_property("intensity", 0.1)

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 1400.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    sky.set_actor_label(f"{GENERATED_PREFIX}DimSkyLight")
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    if component:
        component.set_editor_property("intensity", 0.035)


def main():
    cube_mesh = load_first_asset_checked(CUBE_MESH_PATHS)
    sphere_mesh = load_first_asset_checked(SPHERE_MESH_PATHS)
    material = load_asset_checked(ECHO_WAVE_MATERIAL_PATH)

    open_or_create_level()
    clear_level()
    configure_world_settings()
    build_echo_atrium_layout(cube_mesh, material)
    add_player_starts()
    add_pickup_markers(sphere_mesh)
    add_orientation_lights()

    unreal.EditorLevelLibrary.save_current_level()
    log("Saved DM_EchoAtrium multi-layer indoor deathmatch map.")


if __name__ == "__main__":
    main()
