import math
from datetime import datetime

import unreal


LEVEL_PATH = "/Game/Maps/LeveL1"
BACKUP_BASE_PATH = "/Game/Maps/LeveL1_Backup_AutoBeforeRebuild"

CUBE_MESH_PATHS = [
    "/Engine/BasicShapes/Cube.Cube",
    "/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
]
BLACK_MATERIAL_PATH = "/Game/Materials/M_Env_Black.M_Env_Black"
FRAGMENT_BLUEPRINT_PATH = "/Game/BluePrints/BP_AcousticFragment.BP_AcousticFragment"
ENERGY_BLUEPRINT_PATH = "/Game/BluePrints/BP_EnergyPickup.BP_EnergyPickup"
EXIT_BLUEPRINT_PATH = "/Game/BluePrints/BP_EchoExit.BP_EchoExit"

GENERATED_PREFIX = "L1_EchoCalibration_"


def log(message):
    unreal.log(f"[Level1EchoCalibration] {message}")


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


def backup_level():
    if not unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        raise RuntimeError(f"Level does not exist: {LEVEL_PATH}")

    backup_path = make_unique_backup_path()
    if not unreal.EditorAssetLibrary.duplicate_asset(LEVEL_PATH, backup_path):
        raise RuntimeError(f"Failed to create level backup: {backup_path}")

    unreal.EditorAssetLibrary.save_asset(backup_path, only_if_is_dirty=False)
    log(f"Backed up {LEVEL_PATH} to {backup_path}")


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


def spawn_static_box(label, location, scale, mesh, material):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    actor.set_actor_scale3d(unreal.Vector(*scale))

    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    if material is not None:
        component.set_material(0, material)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_collision_profile_name("BlockAll")
    try:
        component.set_collision_response_to_all_channels(unreal.CollisionResponse.BLOCK)
    except Exception as exc:
        log(f"BlockAll profile applied; skipped explicit channel responses on {label}: {exc}")
    return actor


def spawn_blueprint(label, cls, location, rotation=(0.0, 0.0, 0.0)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls,
        unreal.Vector(*location),
        unreal.Rotator(*rotation),
    )
    actor.set_actor_label(f"{GENERATED_PREFIX}{label}")
    return actor


def set_collectible_visible_for_level1(actor):
    if actor is None:
        return

    actor.set_actor_scale3d(unreal.Vector(1.5, 1.5, 1.5))
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
        else:
            log("Skipped FrequencyAffinity override: EEchoFrequencyAffinity was not available in Python.")

    mesh_component = actor.get_component_by_class(unreal.StaticMeshComponent)
    if mesh_component is not None:
        mesh_component.set_hidden_in_game(False)
        mesh_component.set_visibility(True, True)


def ensure_player_start():
    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-1700.0, -900.0, 120.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    start.set_actor_label(f"{GENERATED_PREFIX}PlayerStart")
    return start


def set_actor_property_if_present(actor, property_name, value):
    try:
        actor.set_editor_property(property_name, value)
        return True
    except Exception as exc:
        object_name = actor.get_name() if hasattr(actor, "get_name") else str(actor)
        log(f"Skipped property {property_name} on {object_name}: {exc}")
        return False


def build_graybox(mesh, material):
    # Unreal starter cube is 100 uu, so scale values map to dimensions / 100.
    spawn_static_box("Floor_Blocking", (0.0, 0.0, -55.0), (44.0, 32.0, 1.0), mesh, material)

    walls = [
        ("Outer_West", (-2200.0, 0.0, 150.0), (1.2, 32.0, 3.0)),
        ("Outer_East", (2200.0, 0.0, 150.0), (1.2, 32.0, 3.0)),
        ("Outer_South", (0.0, -1600.0, 150.0), (44.0, 1.2, 3.0)),
        ("Outer_North", (0.0, 1600.0, 150.0), (44.0, 1.2, 3.0)),
        ("Start_NorthGuide", (-1350.0, -500.0, 150.0), (14.0, 1.2, 3.0)),
        ("Start_SouthGuide", (-900.0, -1180.0, 150.0), (16.0, 1.2, 3.0)),
        ("Central_Block_A", (-350.0, -250.0, 150.0), (1.2, 16.0, 3.0)),
        ("Central_Block_B", (350.0, 250.0, 150.0), (1.2, 16.0, 3.0)),
        ("UTurn_South", (700.0, -850.0, 150.0), (16.0, 1.2, 3.0)),
        ("UTurn_North", (900.0, 850.0, 150.0), (14.0, 1.2, 3.0)),
        ("Exit_Funnel_West", (1350.0, 300.0, 150.0), (1.2, 11.0, 3.0)),
        ("Exit_Funnel_South", (1650.0, 500.0, 150.0), (8.0, 1.2, 3.0)),
    ]

    for label, location, scale in walls:
        spawn_static_box(label, location, scale, mesh, material)


def build_pickups_and_exit(fragment_cls, energy_cls, exit_cls):
    fragments = [
        ("Fragment_01_Tutorial", (-600.0, -650.0, 160.0)),
        ("Fragment_02_Branch", (450.0, 650.0, 160.0)),
        ("Fragment_03_ExitKey", (1450.0, -350.0, 160.0)),
    ]
    for label, location in fragments:
        set_collectible_visible_for_level1(spawn_blueprint(label, fragment_cls, location))

    energies = [
        ("Energy_01_Route", (100.0, -850.0, 160.0)),
        ("Energy_02_NorthReward", (1050.0, 650.0, 160.0)),
    ]
    for label, location in energies:
        set_collectible_visible_for_level1(spawn_blueprint(label, energy_cls, location))

    exit_actor = spawn_blueprint("Exit", exit_cls, (1850.0, 900.0, 120.0))
    set_actor_property_if_present(exit_actor, "required_fragments_override", 3)
    set_actor_property_if_present(exit_actor, "next_level_name", unreal.Name(""))


def add_orientation_lights():
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 1200.0),
        unreal.Rotator(-60.0, 0.0, 0.0),
    )
    light.set_actor_label(f"{GENERATED_PREFIX}DimDirectionalLight")
    component = light.get_component_by_class(unreal.DirectionalLightComponent)
    if component:
        component.set_editor_property("intensity", 0.15)

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 600.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    sky.set_actor_label(f"{GENERATED_PREFIX}DimSkyLight")
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    if component:
        component.set_editor_property("intensity", 0.05)


def main():
    mesh = load_first_asset_checked(CUBE_MESH_PATHS)
    material = load_asset_checked(BLACK_MATERIAL_PATH)
    fragment_cls = load_blueprint_generated_class_checked(FRAGMENT_BLUEPRINT_PATH)
    energy_cls = load_blueprint_generated_class_checked(ENERGY_BLUEPRINT_PATH)
    exit_cls = load_blueprint_generated_class_checked(EXIT_BLUEPRINT_PATH)

    backup_level()
    unreal.EditorLoadingAndSavingUtils.load_map(LEVEL_PATH)
    clear_level()

    ensure_player_start()
    build_graybox(mesh, material)
    build_pickups_and_exit(fragment_cls, energy_cls, exit_cls)
    add_orientation_lights()

    unreal.EditorLevelLibrary.save_current_level()
    log("Saved rebuilt Level 1 Echo Calibration map.")


if __name__ == "__main__":
    main()
