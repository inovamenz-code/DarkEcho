import unreal


MPC_PATH = "/Game/Materials/MPC_EchoWaves"
TARGET_SLOT_COUNT = 16

VECTOR_PREFIXES = (
    "EchoOrigin",
)

SCALAR_PREFIXES = (
    "EchoStartTime",
    "EchoSpeed",
    "EchoWidth",
    "EchoIntensity",
    "EchoDuration",
    "EchoMaxRadius",
)

BACKUP_PATH = "/Game/Materials/MPC_EchoWaves_Backup_AutoBefore16Slots"


def log(message):
    unreal.log(f"[EchoWaveMPCSlots] {message}")


def load_asset_checked(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def get_parameter_name(parameter):
    return str(parameter.get_editor_property("parameter_name"))


def make_scalar_parameter(name):
    parameter = unreal.CollectionScalarParameter()
    parameter.set_editor_property("parameter_name", name)
    parameter.set_editor_property("default_value", 0.0)
    return parameter


def make_vector_parameter(name):
    parameter = unreal.CollectionVectorParameter()
    parameter.set_editor_property("parameter_name", name)
    parameter.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    return parameter


def expected_names(prefixes):
    return [f"{prefix}{slot_index}" for slot_index in range(TARGET_SLOT_COUNT) for prefix in prefixes]


def append_missing_parameters(parameters, prefixes, factory):
    existing_names = {get_parameter_name(parameter) for parameter in parameters}
    added_names = []

    for name in expected_names(prefixes):
        if name in existing_names:
            continue

        parameters.append(factory(name))
        existing_names.add(name)
        added_names.append(name)

    return added_names


def main():
    mpc = load_asset_checked(MPC_PATH)

    scalar_parameters = list(mpc.get_editor_property("scalar_parameters"))
    vector_parameters = list(mpc.get_editor_property("vector_parameters"))

    added_scalars = append_missing_parameters(scalar_parameters, SCALAR_PREFIXES, make_scalar_parameter)
    added_vectors = append_missing_parameters(vector_parameters, VECTOR_PREFIXES, make_vector_parameter)

    if not added_scalars and not added_vectors:
        log(f"{MPC_PATH} already has all Echo wave slots 0-{TARGET_SLOT_COUNT - 1}.")
        return

    if unreal.EditorAssetLibrary.does_asset_exist(BACKUP_PATH):
        log(f"Using existing backup: {BACKUP_PATH}")
    else:
        if not unreal.EditorAssetLibrary.duplicate_asset(MPC_PATH, BACKUP_PATH):
            raise RuntimeError(f"Failed to create backup: {BACKUP_PATH}")

        unreal.EditorAssetLibrary.save_asset(BACKUP_PATH, only_if_is_dirty=False)
        log(f"Backed up {MPC_PATH} to {BACKUP_PATH}")

    mpc.modify()
    mpc.set_editor_property("scalar_parameters", scalar_parameters)
    mpc.set_editor_property("vector_parameters", vector_parameters)

    if not unreal.EditorAssetLibrary.save_asset(MPC_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save asset: {MPC_PATH}")

    log(f"Added {len(added_vectors)} vector parameters: {', '.join(added_vectors)}")
    log(f"Added {len(added_scalars)} scalar parameters: {', '.join(added_scalars)}")
    log(f"Saved {MPC_PATH}")


main()
