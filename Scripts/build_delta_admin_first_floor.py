from datetime import datetime
import math

import unreal


LEVEL_PATH = "/Game/Maps/Delta_Admin_1F"
BACKUP_BASE_PATH = "/Game/Maps/Delta_Admin_1F_Backup_AutoBeforeRebuild"

CUBE_MESH_PATHS = [
    "/Engine/BasicShapes/Cube.Cube",
    "/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
]

MATERIAL_PACKAGE_PATH = "/Game/Materials"
ECHO_WAVE_MATERIAL_PATH = "/Game/Materials/M_Env_EchoWave_Test.M_Env_EchoWave_Test"

MATERIALS = {
    "green_marker": ("M_DeltaAdmin_GreenDoor", unreal.LinearColor(0.02, 0.62, 0.27, 1.0)),
    "red_marker": ("M_DeltaAdmin_RedDoor", unreal.LinearColor(0.86, 0.03, 0.03, 1.0)),
}

FALLBACK_MATERIAL_PATHS = [
    "/Game/Materials/M_Env_Black.M_Env_Black",
    "/Game/LevelPrototyping/Materials/M_PrototypeGrid.M_PrototypeGrid",
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
]

DEATHMATCH_GAME_MODE_BLUEPRINT_PATH = "/Game/BluePrints/BP_EchoDeathmatchGameMode.BP_EchoDeathmatchGameMode"

GENERATED_PREFIX = "DeltaAdmin1F_"
ECHO_DOOR_COLLISION_PROFILE = "EchoDoorBarrier"

# Source drawing is 2D image-space: X right, Y down.
SOURCE_CENTER_X = 725.0
SOURCE_CENTER_Y = 350.0
XY_SCALE = 20.0
WALL_HEIGHT = 900.0
WALL_THICKNESS = 40.0
FLOOR_THICKNESS = 50.0
DOOR_BARRIER_HEIGHT = 840.0
DOOR_BARRIER_THICKNESS = 14.0
DOOR_MARKER_HEIGHT = 36.0
DOOR_MARKER_THICKNESS = 18.0
DOOR_MARKER_BASE_Z = DOOR_BARRIER_HEIGHT - DOOR_MARKER_HEIGHT
ARROW_HEIGHT = 12.0
TARGET_STAIR_RISER_HEIGHT = 15.0


OUTDOOR_GROUNDS = [
    ("Courtyard_North", (0, 25, 1450, 90)),
    ("Courtyard_South", (0, 585, 1450, 90)),
    ("Courtyard_West", (0, 115, 90, 470)),
    ("Courtyard_Center", (780, 115, 120, 470)),
    ("Courtyard_East", (1360, 115, 90, 470)),
]

FOOTPRINTS = [
    ("LeftFloor", (90, 115, 690, 470)),
    ("RightFloor", (900, 115, 460, 470)),
]

GREEN_DOORS = [
    ("D1", (90, 445, 90, 520)),
    ("D2", (250, 585, 340, 585)),
    ("D3", (680, 115, 750, 115)),
    ("D5", (560, 390, 560, 450)),
    ("D7", (660, 390, 725, 390)),
    ("D8", (780, 330, 780, 385)),
    ("D9", (945, 115, 980, 115)),
    ("D10", (1035, 250, 1110, 250)),
    ("D11", (1215, 250, 1280, 250)),
    ("D12", (1185, 305, 1185, 365)),
    ("D13", (1115, 470, 1180, 470)),
    ("D14", (1280, 470, 1280, 535)),
    ("D15", (1360, 445, 1360, 520)),
    ("D16", (900, 330, 900, 385)),
]

RED_DOORS = [
    ("R1", (1295, 250, 1345, 250), "down"),
    ("R2", (1120, 385, 1175, 385), "up"),
    ("R3", (455, 460, 455, 520), "right"),
    ("R4", (360, 390, 410, 390), "down"),
]

OUTER_WALLS = [
    ("Outer_Left_West", (90, 115, 90, 585)),
    ("Outer_Left_North", (90, 115, 780, 115)),
    ("Outer_Left_East", (780, 115, 780, 585)),
    ("Outer_Left_South", (90, 585, 780, 585)),
    ("Outer_Right_West", (900, 115, 900, 585)),
    ("Outer_Right_North", (900, 115, 1360, 115)),
    ("Outer_Right_East", (1360, 115, 1360, 585)),
    ("Outer_Right_South", (900, 585, 1360, 585)),
]

PERIMETER_WALLS = [
    ("Perimeter_West", (0, 25, 0, 675)),
    ("Perimeter_North", (0, 25, 1450, 25)),
    ("Perimeter_East", (1450, 25, 1450, 675)),
    ("Perimeter_South", (0, 675, 1450, 675)),
]

INNER_WALLS = [
    ("W1", (190, 115, 190, 250)),
    ("W2", (320, 115, 320, 255)),
    ("W3", (455, 115, 455, 220)),
    ("W4", (560, 115, 560, 585)),
    ("W5", (660, 115, 660, 245)),
    ("W7", (560, 390, 780, 390)),
    ("W8", (560, 475, 707, 475)),
    ("W9", (90, 310, 190, 310)),
    ("W10", (90, 470, 190, 470)),
    ("W11", (190, 390, 455, 390)),
    ("W12", (455, 290, 560, 290)),
    ("W14", (455, 390, 455, 585)),
    ("W15", (305, 395, 305, 585)),
    ("W16", (980, 115, 980, 250)),
    ("W17", (1135, 115, 1135, 250)),
    ("W18_A", (1280, 115, 1280, 145)),
    ("W18_B", (1280, 180, 1280, 250)),
    ("W19", (900, 250, 1280, 250)),
    ("W21", (900, 385, 1360, 385)),
    ("W22", (990, 250, 990, 585)),
    ("W24", (1280, 385, 1280, 585)),
    ("W26", (1115, 470, 1280, 470)),
    ("W27", (1185, 250, 1185, 385)),
]

STAIRS = [
    ("S1", (105, 205, 80, 150)),
    ("S2", (910, 400, 70, 135)),
    ("S3", (1288, 420, 62, 120)),
]


def log(message):
    unreal.log(f"[DeltaAdmin1F] {message}")


def source_to_world(x, y, z=0.0):
    return (
        (float(x) - SOURCE_CENTER_X) * XY_SCALE,
        (SOURCE_CENTER_Y - float(y)) * XY_SCALE,
        float(z),
    )


def source_segment_center(segment):
    x1, y1, x2, y2 = segment
    return ((x1 + x2) * 0.5, (y1 + y2) * 0.5)


def is_horizontal(segment):
    return segment[1] == segment[3]


def is_vertical(segment):
    return segment[0] == segment[2]


def normalized_axis_range(segment):
    if is_horizontal(segment):
        return tuple(sorted((segment[0], segment[2])))
    if is_vertical(segment):
        return tuple(sorted((segment[1], segment[3])))
    raise ValueError(f"Only axis-aligned segments are supported: {segment}")


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


def load_first_available_material():
    for path in FALLBACK_MATERIAL_PATHS:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset is not None:
            log(f"Using fallback material: {path}")
            return asset
    return None


def load_blueprint_generated_class(path):
    blueprint = unreal.EditorAssetLibrary.load_asset(path)
    if blueprint is None:
        return None
    generated_class = blueprint.generated_class()
    if generated_class is None:
        log(f"Blueprint has no generated class: {path}")
    return generated_class


def create_color_material(asset_name, color, fallback):
    asset_path = f"{MATERIAL_PACKAGE_PATH}/{asset_name}.{asset_name}"
    package_asset_path = f"{MATERIAL_PACKAGE_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(package_asset_path):
        existing = unreal.EditorAssetLibrary.load_asset(asset_path)
        if existing is not None:
            return existing

    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        material = asset_tools.create_asset(
            asset_name,
            MATERIAL_PACKAGE_PATH,
            unreal.Material,
            unreal.MaterialFactoryNew(),
        )
        if material is None:
            raise RuntimeError("create_asset returned None")

        base_color = unreal.MaterialEditingLibrary.create_material_expression(
            material,
            unreal.MaterialExpressionConstant3Vector,
            -420,
            -120,
        )
        base_color.set_editor_property("constant", color)
        unreal.MaterialEditingLibrary.connect_material_property(
            base_color,
            "",
            unreal.MaterialProperty.MP_BASE_COLOR,
        )

        roughness = unreal.MaterialEditingLibrary.create_material_expression(
            material,
            unreal.MaterialExpressionConstant,
            -420,
            80,
        )
        roughness.set_editor_property("r", 0.85)
        unreal.MaterialEditingLibrary.connect_material_property(
            roughness,
            "",
            unreal.MaterialProperty.MP_ROUGHNESS,
        )

        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_asset(package_asset_path, only_if_is_dirty=False)
        log(f"Created material: {asset_path}")
        return material
    except Exception as exc:
        log(f"Could not create material {asset_name}; using fallback. {exc}")
        return fallback


def load_or_create_materials():
    fallback = load_first_available_material()
    echo_wave_material = load_asset_checked(ECHO_WAVE_MATERIAL_PATH)
    result = {
        "wall": echo_wave_material,
        "floor": echo_wave_material,
        "stair": echo_wave_material,
        "echo_barrier": echo_wave_material,
    }
    for key, (asset_name, color) in MATERIALS.items():
        result[key] = create_color_material(asset_name, color, fallback)
    return result


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


def get_collision_channel(*names):
    channel_enum = getattr(unreal, "CollisionChannel", None)
    if channel_enum is None:
        return None

    for name in names:
        value = getattr(channel_enum, name, None)
        if value is not None:
            return value
    return None


def get_collision_response(*names):
    response_enum = getattr(unreal, "CollisionResponse", None)
    if response_enum is None:
        return None

    for name in names:
        value = getattr(response_enum, name, None)
        if value is not None:
            return value
    return None


def set_component_response(component, channel, response):
    if channel is None or response is None:
        return False
    try:
        component.set_collision_response_to_channel(channel, response)
        return True
    except Exception as exc:
        log(f"Skipped collision channel response setup: {exc}")
        return False


def configure_blocking_collision(component):
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_collision_profile_name("BlockAll")
    block_response = get_collision_response("ECR_BLOCK", "ECR_Block", "BLOCK", "Block")
    try:
        if block_response is not None:
            component.set_collision_response_to_all_channels(block_response)
    except Exception as exc:
        log(f"BlockAll profile applied; skipped explicit wall responses: {exc}")


def configure_echo_barrier_collision(component):
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
    try:
        component.set_collision_profile_name(ECHO_DOOR_COLLISION_PROFILE)
    except Exception as exc:
        log(f"Could not apply {ECHO_DOOR_COLLISION_PROFILE} collision profile; using explicit channel fallback. {exc}")

    ignore_response = get_collision_response("ECR_IGNORE", "ECR_Ignore", "IGNORE", "Ignore")
    block_response = get_collision_response("ECR_BLOCK", "ECR_Block", "BLOCK", "Block")

    if ignore_response is None or block_response is None:
        log(f"{ECHO_DOOR_COLLISION_PROFILE} profile applied; skipped explicit response fallback.")
        return

    try:
        component.set_collision_response_to_all_channels(ignore_response)
    except Exception as exc:
        log(f"Skipped all-channel ignore setup for echo barrier: {exc}")

    visibility = get_collision_channel("ECC_VISIBILITY", "ECC_Visibility", "VISIBILITY", "Visibility")
    camera = get_collision_channel("ECC_CAMERA", "ECC_Camera", "CAMERA", "Camera")
    pawn = get_collision_channel("ECC_PAWN", "ECC_Pawn", "PAWN", "Pawn")
    world_static = get_collision_channel("ECC_WORLD_STATIC", "ECC_WorldStatic", "WORLD_STATIC", "WorldStatic")

    set_component_response(component, visibility, block_response)
    set_component_response(component, camera, block_response)
    set_component_response(component, world_static, block_response)
    set_component_response(component, pawn, ignore_response)


def configure_no_collision(component):
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)


def spawn_static_box(label, location, size, mesh, material, tag=None, collision_mode="block", rotation=(0.0, 0.0, 0.0)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(*rotation),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    if tag:
        actor.tags = [unreal.Name(tag)]

    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    if material is not None:
        component.set_material(0, material)

    if collision_mode == "block":
        configure_blocking_collision(component)
    elif collision_mode == "echo_barrier":
        configure_echo_barrier_collision(component)
    else:
        configure_no_collision(component)

    return actor


def spawn_box_from_source_rect(label, rect, z, height, mesh, material, tag, collision_mode="block"):
    x, y, width, depth = rect
    center_x = x + width * 0.5
    center_y = y + depth * 0.5
    world_x, world_y, _ = source_to_world(center_x, center_y)
    spawn_static_box(
        label,
        (world_x, world_y, z + height * 0.5),
        (width * XY_SCALE, depth * XY_SCALE, height),
        mesh,
        material,
        tag=tag,
        collision_mode=collision_mode,
    )


def spawn_segment_box(
    label,
    segment,
    height,
    mesh,
    material,
    tag="EchoMapWall",
    collision_mode="block",
    thickness=WALL_THICKNESS,
    z_base=0.0,
):
    x1, y1, x2, y2 = segment
    center_x, center_y = source_segment_center(segment)
    world_x, world_y, _ = source_to_world(center_x, center_y)

    if is_horizontal(segment):
        length = abs(x2 - x1) * XY_SCALE
        size = (length, thickness, height)
    elif is_vertical(segment):
        length = abs(y2 - y1) * XY_SCALE
        size = (thickness, length, height)
    else:
        raise ValueError(f"Only axis-aligned segments are supported: {segment}")

    if size[0] <= 0.0 or size[1] <= 0.0:
        log(f"Skipped zero-sized segment {label}: {segment}")
        return None

    return spawn_static_box(
        label,
        (world_x, world_y, z_base + height * 0.5),
        size,
        mesh,
        material,
        tag=tag,
        collision_mode=collision_mode,
    )


def colinear_cut_range(wall_segment, door_segment):
    if is_horizontal(wall_segment) and is_horizontal(door_segment) and wall_segment[1] == door_segment[1]:
        wall_min, wall_max = normalized_axis_range(wall_segment)
        door_min, door_max = normalized_axis_range(door_segment)
    elif is_vertical(wall_segment) and is_vertical(door_segment) and wall_segment[0] == door_segment[0]:
        wall_min, wall_max = normalized_axis_range(wall_segment)
        door_min, door_max = normalized_axis_range(door_segment)
    else:
        return None

    cut_min = max(wall_min, door_min)
    cut_max = min(wall_max, door_max)
    if cut_min >= cut_max:
        return None
    return (cut_min, cut_max)


def split_wall_for_doors(wall_segment, door_segments):
    wall_min, wall_max = normalized_axis_range(wall_segment)
    cuts = []
    for door_segment in door_segments:
        cut = colinear_cut_range(wall_segment, door_segment)
        if cut is not None:
            cuts.append(cut)

    if not cuts:
        return [wall_segment]

    cuts.sort()
    merged_cuts = []
    for cut_min, cut_max in cuts:
        if not merged_cuts or cut_min > merged_cuts[-1][1]:
            merged_cuts.append([cut_min, cut_max])
        else:
            merged_cuts[-1][1] = max(merged_cuts[-1][1], cut_max)

    spans = []
    cursor = wall_min
    for cut_min, cut_max in merged_cuts:
        if cursor < cut_min:
            spans.append((cursor, cut_min))
        cursor = max(cursor, cut_max)
    if cursor < wall_max:
        spans.append((cursor, wall_max))

    result = []
    if is_horizontal(wall_segment):
        y = wall_segment[1]
        for span_min, span_max in spans:
            result.append((span_min, y, span_max, y))
    else:
        x = wall_segment[0]
        for span_min, span_max in spans:
            result.append((x, span_min, x, span_max))
    return result


def build_floors(mesh, materials):
    for label, rect in OUTDOOR_GROUNDS:
        spawn_box_from_source_rect(
            label,
            rect,
            -FLOOR_THICKNESS,
            FLOOR_THICKNESS,
            mesh,
            materials["floor"],
            tag="EchoMapFloor",
            collision_mode="block",
        )

    for label, rect in FOOTPRINTS:
        spawn_box_from_source_rect(
            label,
            rect,
            -FLOOR_THICKNESS,
            FLOOR_THICKNESS,
            mesh,
            materials["floor"],
            tag="EchoMapFloor",
            collision_mode="block",
        )


def build_walls(mesh, materials):
    all_doors = [segment for _, segment in GREEN_DOORS] + [segment for _, segment, _ in RED_DOORS]
    all_walls = PERIMETER_WALLS + OUTER_WALLS + INNER_WALLS
    for wall_label, wall_segment in all_walls:
        pieces = split_wall_for_doors(wall_segment, all_doors)
        for index, piece in enumerate(pieces):
            piece_label = wall_label if len(pieces) == 1 else f"{wall_label}_{chr(ord('A') + index)}"
            spawn_segment_box(piece_label, piece, WALL_HEIGHT, mesh, materials["wall"], tag="EchoMapWall")


def build_echo_doors(mesh, materials):
    for door_id, segment in GREEN_DOORS:
        spawn_segment_box(
            f"{door_id}_EchoDoorBarrier",
            segment,
            DOOR_BARRIER_HEIGHT,
            mesh,
            materials["echo_barrier"],
            tag="EchoDoorBarrier",
            collision_mode="echo_barrier",
            thickness=DOOR_BARRIER_THICKNESS,
        )
        spawn_door_color_marker(door_id, segment, mesh, materials["green_marker"], "EchoDoorColorMarker")

    for door_id, segment, direction in RED_DOORS:
        spawn_segment_box(
            f"{door_id}_OneWayEchoBarrier",
            segment,
            DOOR_BARRIER_HEIGHT,
            mesh,
            materials["echo_barrier"],
            tag="EchoOneWayDoorBarrier",
            collision_mode="echo_barrier",
            thickness=DOOR_BARRIER_THICKNESS,
        )
        spawn_door_color_marker(door_id, segment, mesh, materials["red_marker"], "EchoOneWayDoorColorMarker")
        spawn_direction_arrow(door_id, segment, direction, mesh, materials["red_marker"])


def spawn_door_color_marker(label, segment, mesh, material, tag):
    spawn_segment_box(
        f"{label}_ColorMarker",
        segment,
        DOOR_MARKER_HEIGHT,
        mesh,
        material,
        tag=tag,
        collision_mode="none",
        thickness=DOOR_MARKER_THICKNESS,
        z_base=DOOR_MARKER_BASE_Z,
    )


def source_direction_to_world(direction):
    if direction == "up":
        return (0.0, 1.0)
    if direction == "down":
        return (0.0, -1.0)
    if direction == "left":
        return (-1.0, 0.0)
    if direction == "right":
        return (1.0, 0.0)
    raise ValueError(f"Unsupported direction: {direction}")


def spawn_direction_arrow(label, segment, direction, mesh, material):
    center_x, center_y = source_segment_center(segment)
    world_x, world_y, _ = source_to_world(center_x, center_y)
    dir_x, dir_y = source_direction_to_world(direction)
    yaw = math.degrees(math.atan2(dir_y, dir_x))

    shaft_length = 320.0
    shaft_width = 45.0
    head_length = 130.0
    head_width = 170.0
    z = ARROW_HEIGHT * 0.5 + 4.0

    shaft_center = (
        world_x + dir_x * 120.0,
        world_y + dir_y * 120.0,
        z,
    )
    head_center = (
        world_x + dir_x * 300.0,
        world_y + dir_y * 300.0,
        z,
    )

    spawn_static_box(
        f"{label}_DirectionArrow_Shaft",
        shaft_center,
        (shaft_length, shaft_width, ARROW_HEIGHT),
        mesh,
        material,
        tag="EchoOneWayDoorArrow",
        collision_mode="none",
        rotation=(0.0, yaw, 0.0),
    )
    spawn_static_box(
        f"{label}_DirectionArrow_Head",
        head_center,
        (head_length, head_width, ARROW_HEIGHT),
        mesh,
        material,
        tag="EchoOneWayDoorArrow",
        collision_mode="none",
        rotation=(0.0, yaw, 0.0),
    )


def build_stairs(mesh, materials):
    for stair_id, rect in STAIRS:
        spawn_stair(stair_id, rect, mesh, materials["stair"])


def spawn_stair(stair_id, rect, mesh, material):
    x, y, width, depth = rect
    steps = max(8, int(math.ceil(WALL_HEIGHT / TARGET_STAIR_RISER_HEIGHT)))
    step_depth = depth / steps
    for index in range(steps):
        # The lower step starts at the bottom/south side of the source rectangle.
        y_max = y + depth - index * step_depth
        y_min = y + depth - (index + 1) * step_depth
        step_height = WALL_HEIGHT * (index + 1) / steps
        step_rect = (x, y_min, width, step_depth)
        spawn_box_from_source_rect(
            f"{stair_id}_Step_{index + 1:02d}",
            step_rect,
            0.0,
            step_height,
            mesh,
            material,
            tag="EchoMapStair",
            collision_mode="block",
        )


def spawn_player_start():
    x, y, z = source_to_world(220, 520, 120.0)
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(x, y, z),
        unreal.Rotator(0.0, 15.0, 0.0),
    )
    start.set_actor_label(f"{GENERATED_PREFIX}PlayerStart")
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
    if deathmatch_class is not None and set_actor_property_if_present(world_settings, "default_game_mode", deathmatch_class):
        log("Set World Settings default_game_mode to C++ EchoDeathmatchGameMode fallback.")


def add_orientation_lights():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 2200.0),
        unreal.Rotator(-60.0, 35.0, 0.0),
    )
    light.set_actor_label(f"{GENERATED_PREFIX}DirectionalLight")
    component = light.get_component_by_class(unreal.DirectionalLightComponent)
    if component:
        component.set_editor_property("intensity", 1.35)

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 1200.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    sky.set_actor_label(f"{GENERATED_PREFIX}SkyLight")
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    if component:
        component.set_editor_property("intensity", 0.6)


def main():
    cube_mesh = load_first_asset_checked(CUBE_MESH_PATHS)
    materials = load_or_create_materials()

    open_or_create_level()
    clear_level()
    configure_world_settings()
    build_floors(cube_mesh, materials)
    build_walls(cube_mesh, materials)
    build_echo_doors(cube_mesh, materials)
    build_stairs(cube_mesh, materials)
    spawn_player_start()
    add_orientation_lights()

    unreal.EditorLevelLibrary.save_current_level()
    log("Saved Delta_Admin_1F graybox map.")


if __name__ == "__main__":
    main()
