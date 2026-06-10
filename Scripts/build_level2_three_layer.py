import math
from datetime import datetime

import unreal


LEVEL_PATH = "/Game/Maps/Level2"
BACKUP_BASE_PATH = "/Game/Maps/Level2_Backup_AutoBeforeRebuild"

CUBE_MESH_PATHS = [
    "/Engine/BasicShapes/Cube.Cube",
    "/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
]
BLACK_MATERIAL_PATH = "/Game/Materials/M_Env_Black.M_Env_Black"
FRAGMENT_BLUEPRINT_PATH = "/Game/BluePrints/BP_AcousticFragment.BP_AcousticFragment"
ENERGY_BLUEPRINT_PATH = "/Game/BluePrints/BP_EnergyPickup.BP_EnergyPickup"
EXIT_BLUEPRINT_PATH = "/Game/BluePrints/BP_EchoExit.BP_EchoExit"

GENERATED_PREFIX = "L2_ThreeLayer_"


def log(message):
    unreal.log(f"[Level2ThreeLayer] {message}")


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


def load_blueprint_generated_class_checked(path):
    blueprint = unreal.EditorAssetLibrary.load_asset(path)
    if blueprint is None:
        raise RuntimeError(f"Missing blueprint asset: {path}")

    cls = blueprint.generated_class()
    if cls is None:
        raise RuntimeError(f"Missing generated class for blueprint: {path}")
    return cls


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


def spawn_static_box(label, location, scale, mesh, material, rotation=(0.0, 0.0, 0.0)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(*rotation),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))

    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    if material is not None:
        component.set_material(0, material)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_collision_profile_name("BlockAll")
    return actor


def spawn_blueprint(label, cls, location, rotation=(0.0, 0.0, 0.0)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls,
        unreal.Vector(*location),
        unreal.Rotator(*rotation),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    return actor


def make_collectible_visible(actor):
    if actor is None:
        return

    actor.set_actor_scale3d(unreal.Vector(1.45, 1.45, 1.45))
    actor.set_actor_hidden_in_game(False)

    reveal_target = actor.get_component_by_class(unreal.EchoRevealTargetComponent)
    if reveal_target is not None:
        set_actor_property_if_present(reveal_target, "b_hide_owner_until_revealed", False)
        set_actor_property_if_present(reveal_target, "hide_owner_until_revealed", False)
        affinity_enum = getattr(unreal, "EEchoFrequencyAffinity", None)
        if affinity_enum is not None:
            both_affinity = getattr(affinity_enum, "BOTH", None)
            if both_affinity is None:
                both_affinity = getattr(affinity_enum, "Both", None)
            if both_affinity is not None:
                set_actor_property_if_present(reveal_target, "frequency_affinity", both_affinity)

    mesh_component = actor.get_component_by_class(unreal.StaticMeshComponent)
    if mesh_component is not None:
        mesh_component.set_hidden_in_game(False)
        mesh_component.set_visibility(True, True)


def ensure_player_start():
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-1650.0, -950.0, 120.0),
        unreal.Rotator(0.0, 35.0, 0.0),
    )
    start.set_actor_label(f"{GENERATED_PREFIX}PlayerStart")
    return start


def spawn_floor_and_outer_walls(mesh, material, layer_name, z, half_x=1900.0, half_y=1350.0):
    spawn_static_box(
        f"{layer_name}_Floor",
        (0.0, 0.0, z - 55.0),
        (half_x * 2.0 / 100.0, half_y * 2.0 / 100.0, 1.0),
        mesh,
        material,
    )

    wall_z = z + 150.0
    spawn_static_box(f"{layer_name}_Outer_West", (-half_x, 0.0, wall_z), (1.2, half_y * 2.0 / 100.0, 3.0), mesh, material)
    spawn_static_box(f"{layer_name}_Outer_East", (half_x, 0.0, wall_z), (1.2, half_y * 2.0 / 100.0, 3.0), mesh, material)
    spawn_static_box(f"{layer_name}_Outer_South", (0.0, -half_y, wall_z), (half_x * 2.0 / 100.0, 1.2, 3.0), mesh, material)
    spawn_static_box(f"{layer_name}_Outer_North", (0.0, half_y, wall_z), (half_x * 2.0 / 100.0, 1.2, 3.0), mesh, material)


def spawn_layer_walls(mesh, material):
    layer_walls = [
        # Layer 1: short route with a visible first ramp reveal.
        ("L1_Start_NorthGuide", (-1180.0, -560.0, 150.0), (13.0, 1.0, 3.0)),
        ("L1_Start_SouthGuide", (-760.0, -1020.0, 150.0), (15.0, 1.0, 3.0)),
        ("L1_FirstTurn_Block", (-230.0, -440.0, 150.0), (1.0, 14.5, 3.0)),
        ("L1_Fragment_Cubby_Back", (-620.0, 620.0, 150.0), (10.0, 1.0, 3.0)),
        ("L1_Fragment_Cubby_Side", (-1120.0, 350.0, 150.0), (1.0, 7.5, 3.0)),
        ("L1_Ramp_Funnel_West", (740.0, 460.0, 150.0), (1.0, 14.0, 3.0)),
        ("L1_Ramp_Funnel_South", (1120.0, 100.0, 150.0), (10.0, 1.0, 3.0)),

        # Layer 2: branchier middle floor, with the fragment off the main ascent route.
        ("L2_LowerLanding_Cover", (760.0, -430.0, 600.0), (1.0, 11.5, 3.0)),
        ("L2_MainSpine_West", (-120.0, -280.0, 600.0), (1.0, 16.0, 3.0)),
        ("L2_MainSpine_East", (480.0, 300.0, 600.0), (1.0, 14.0, 3.0)),
        ("L2_NorthBranch_Gate", (-600.0, 560.0, 600.0), (12.0, 1.0, 3.0)),
        ("L2_Fragment_Cubby_Back", (-1220.0, 850.0, 600.0), (9.5, 1.0, 3.0)),
        ("L2_Fragment_Cubby_Side", (-1520.0, 470.0, 600.0), (1.0, 7.5, 3.0)),
        ("L2_Ramp2_Funnel", (1120.0, 550.0, 600.0), (11.0, 1.0, 3.0)),
        ("L2_Ramp2_Screen", (1380.0, -250.0, 600.0), (1.0, 10.0, 3.0)),

        # Layer 3: open upper floor with staggered occluders and a final exit line.
        ("L3_UpperLanding_Screen", (520.0, -560.0, 1050.0), (1.0, 13.5, 3.0)),
        ("L3_Central_Occluder_A", (-220.0, -240.0, 1050.0), (1.0, 13.0, 3.0)),
        ("L3_Central_Occluder_B", (330.0, 330.0, 1050.0), (1.0, 13.0, 3.0)),
        ("L3_Fragment_Cubby_Back", (-960.0, -820.0, 1050.0), (11.0, 1.0, 3.0)),
        ("L3_Fragment_Cubby_Side", (-1400.0, -500.0, 1050.0), (1.0, 7.0, 3.0)),
        ("L3_Exit_Funnel_West", (1060.0, 220.0, 1050.0), (1.0, 10.0, 3.0)),
        ("L3_Exit_Funnel_South", (1370.0, 610.0, 1050.0), (9.0, 1.0, 3.0)),
    ]

    for label, location, scale in layer_walls:
        spawn_static_box(label, location, scale, mesh, material)


def spawn_ramp(label, start, end, width, thickness, mesh, material):
    start_x, start_y, start_z = start
    end_x, end_y, end_z = end
    dx = end_x - start_x
    dy = end_y - start_y
    dz = end_z - start_z
    horizontal = math.sqrt(dx * dx + dy * dy)
    length = math.sqrt(horizontal * horizontal + dz * dz)
    yaw = math.degrees(math.atan2(dy, dx))
    pitch = math.degrees(math.atan2(dz, horizontal))
    center = ((start_x + end_x) * 0.5, (start_y + end_y) * 0.5, (start_z + end_z) * 0.5)

    spawn_static_box(
        label,
        center,
        (length / 100.0, width / 100.0, thickness / 100.0),
        mesh,
        material,
        (pitch, yaw, 0.0),
    )

    # Low side rails keep the ball on the ramp without hiding the vertical relationship.
    rail_offset_x = -math.sin(math.radians(yaw)) * (width * 0.5 + 45.0)
    rail_offset_y = math.cos(math.radians(yaw)) * (width * 0.5 + 45.0)
    rail_z_offset = 95.0
    rail_scale = (length / 100.0, 0.45, 1.3)
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


def spawn_vertical_connectors(mesh, material):
    spawn_ramp(
        "Ramp_Layer1_To_Layer2",
        (980.0, 700.0, 10.0),
        (1450.0, -980.0, 460.0),
        520.0,
        55.0,
        mesh,
        material,
    )
    spawn_static_box("Ramp1_Lower_Landing", (940.0, 790.0, -25.0), (7.0, 6.0, 0.7), mesh, material)
    spawn_static_box("Ramp1_Upper_Landing", (1450.0, -1040.0, 425.0), (7.5, 6.0, 0.7), mesh, material)

    spawn_ramp(
        "Ramp_Layer2_To_Layer3",
        (1180.0, 820.0, 460.0),
        (1500.0, -720.0, 910.0),
        520.0,
        55.0,
        mesh,
        material,
    )
    spawn_static_box("Ramp2_Lower_Landing", (1120.0, 850.0, 425.0), (7.5, 6.0, 0.7), mesh, material)
    spawn_static_box("Ramp2_Upper_Landing", (1500.0, -760.0, 875.0), (7.5, 6.0, 0.7), mesh, material)


def spawn_sightline_features(mesh, material):
    # Thin overhead bars give each layer echo-readable silhouettes without fully closing routes.
    features = [
        ("L1_ViewToUpper_Pillar_A", (1320.0, 980.0, 300.0), (1.0, 1.0, 7.0)),
        ("L1_ViewToUpper_Pillar_B", (1580.0, 980.0, 300.0), (1.0, 1.0, 7.0)),
        ("L2_VerticalReference_Pillar_A", (1240.0, -950.0, 750.0), (1.0, 1.0, 7.0)),
        ("L2_VerticalReference_Pillar_B", (1540.0, -950.0, 750.0), (1.0, 1.0, 7.0)),
        ("L3_FinalBeacon_Pillar_A", (1500.0, 880.0, 1200.0), (1.0, 1.0, 4.0)),
        ("L3_FinalBeacon_Pillar_B", (1740.0, 880.0, 1200.0), (1.0, 1.0, 4.0)),
    ]
    for label, location, scale in features:
        spawn_static_box(label, location, scale, mesh, material)


def build_graybox(mesh, material):
    spawn_floor_and_outer_walls(mesh, material, "Layer1", 0.0)
    spawn_floor_and_outer_walls(mesh, material, "Layer2", 450.0)
    spawn_floor_and_outer_walls(mesh, material, "Layer3", 900.0)
    spawn_layer_walls(mesh, material)
    spawn_vertical_connectors(mesh, material)
    spawn_sightline_features(mesh, material)


def build_pickups_and_exit(fragment_cls, energy_cls, exit_cls):
    fragments = [
        ("Fragment_01_LowerCove", (-1380.0, 700.0, 160.0)),
        ("Fragment_02_MiddleBranch", (-1470.0, 820.0, 610.0)),
        ("Fragment_03_UpperTurn", (-1420.0, -780.0, 1060.0)),
    ]
    for label, location in fragments:
        make_collectible_visible(spawn_blueprint(label, fragment_cls, location))

    energies = [
        ("Energy_01_RampPreview", (430.0, 820.0, 160.0)),
        ("Energy_02_MiddleMainRoute", (330.0, -840.0, 610.0)),
        ("Energy_03_UpperExitRoute", (850.0, 620.0, 1060.0)),
    ]
    for label, location in energies:
        make_collectible_visible(spawn_blueprint(label, energy_cls, location))

    exit_actor = spawn_blueprint("Exit_TopLayer", exit_cls, (1650.0, 900.0, 1020.0))
    set_actor_property_if_present(exit_actor, "required_fragments_override", 3)
    set_actor_property_if_present(exit_actor, "next_level_name", unreal.Name(""))


def add_orientation_lights():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 1700.0),
        unreal.Rotator(-65.0, 20.0, 0.0),
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
    mesh = load_first_asset_checked(CUBE_MESH_PATHS)
    material = load_asset_checked(BLACK_MATERIAL_PATH)
    fragment_cls = load_blueprint_generated_class_checked(FRAGMENT_BLUEPRINT_PATH)
    energy_cls = load_blueprint_generated_class_checked(ENERGY_BLUEPRINT_PATH)
    exit_cls = load_blueprint_generated_class_checked(EXIT_BLUEPRINT_PATH)

    open_or_create_level()
    clear_level()

    ensure_player_start()
    build_graybox(mesh, material)
    build_pickups_and_exit(fragment_cls, energy_cls, exit_cls)
    add_orientation_lights()

    unreal.EditorLevelLibrary.save_current_level()
    log("Saved rebuilt Level 2 three-layer exploration map.")


if __name__ == "__main__":
    main()
