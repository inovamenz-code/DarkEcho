import unreal


MPC_PATH = "/Game/Materials/MPC_EchoWaves"
MATERIAL_PATH = "/Game/Materials/M_Env_EchoWave_Test"
MATERIAL_BACKUP_PATH = "/Game/Materials/M_Env_EchoWave_Test_Backup_AutoBefore16Slots"
FUNCTION_PATH = "/Game/Materials/MF_EchoWaveSlot"
SLOT_COUNT = 16

VECTOR_PREFIXES = ("EchoOrigin",)
SCALAR_PREFIXES = (
    "EchoStartTime",
    "EchoSpeed",
    "EchoWidth",
    "EchoIntensity",
    "EchoDuration",
    "EchoMaxRadius",
)


def log(message):
    unreal.log(f"[EchoWave16SlotMaterial] {message}")


def load_asset_checked(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def save_asset_checked(path):
    if not unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save asset: {path}")


def parameter_name(parameter):
    return str(parameter.get_editor_property("parameter_name"))


def expected_names(prefixes):
    return [f"{prefix}{slot_index}" for slot_index in range(SLOT_COUNT) for prefix in prefixes]


def validate_mpc_parameters(mpc):
    scalar_names = {parameter_name(parameter) for parameter in mpc.get_editor_property("scalar_parameters")}
    vector_names = {parameter_name(parameter) for parameter in mpc.get_editor_property("vector_parameters")}
    missing_scalars = [name for name in expected_names(SCALAR_PREFIXES) if name not in scalar_names]
    missing_vectors = [name for name in expected_names(VECTOR_PREFIXES) if name not in vector_names]

    if missing_scalars or missing_vectors:
        raise RuntimeError(
            "MPC_EchoWaves is missing parameters. Run Scripts/ensure_echo_wave_mpc_slots.py first. "
            f"Missing vectors: {missing_vectors}; missing scalars: {missing_scalars}"
        )


def duplicate_backup_if_needed(source_path, backup_path):
    if unreal.EditorAssetLibrary.does_asset_exist(backup_path):
        log(f"Using existing backup: {backup_path}")
        return

    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, backup_path):
        raise RuntimeError(f"Failed to create backup: {backup_path}")

    save_asset_checked(backup_path)
    log(f"Backed up {source_path} to {backup_path}")


def get_or_create_material_function():
    if unreal.EditorAssetLibrary.does_asset_exist(FUNCTION_PATH):
        return load_asset_checked(FUNCTION_PATH)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFunctionFactoryNew()
    function = asset_tools.create_asset(
        "MF_EchoWaveSlot",
        "/Game/Materials",
        unreal.MaterialFunction,
        factory,
    )
    if function is None:
        raise RuntimeError(f"Failed to create material function: {FUNCTION_PATH}")

    log(f"Created {FUNCTION_PATH}")
    return function


def set_optional_property(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
    except Exception as exc:
        log(f"Skipped setting {obj.get_name()}.{property_name}: {exc}")


def function_input(material_function, name, input_type, x, y, sort_priority):
    expression = unreal.MaterialEditingLibrary.create_material_expression_in_function(
        material_function,
        unreal.MaterialExpressionFunctionInput,
        x,
        y,
    )
    expression.set_editor_property("input_name", name)
    expression.set_editor_property("input_type", input_type)
    expression.set_editor_property("sort_priority", sort_priority)
    set_optional_property(expression, "use_preview_value_as_default", True)
    return expression


def custom_input(name):
    value = unreal.CustomInput()
    value.set_editor_property("input_name", name)
    return value


def build_echo_wave_slot_function(material_function):
    unreal.MaterialEditingLibrary.delete_all_material_expressions_in_function(material_function)

    vector3 = unreal.FunctionInputType.FUNCTION_INPUT_VECTOR3
    vector4 = unreal.FunctionInputType.FUNCTION_INPUT_VECTOR4
    scalar = unreal.FunctionInputType.FUNCTION_INPUT_SCALAR

    inputs = [
        ("WorldPosition", vector3),
        ("TimeValue", scalar),
        ("Origin", vector4),
        ("StartTime", scalar),
        ("Speed", scalar),
        ("Width", scalar),
        ("WaveIntensity", scalar),
        ("Duration", scalar),
        ("MaxRadius", scalar),
    ]

    input_nodes = {}
    for index, (name, input_type) in enumerate(inputs):
        input_nodes[name] = function_input(
            material_function,
            name,
            input_type,
            -700,
            -360 + index * 90,
            index,
        )

    custom = unreal.MaterialEditingLibrary.create_material_expression_in_function(
        material_function,
        unreal.MaterialExpressionCustom,
        -260,
        0,
    )
    custom.set_editor_property("description", "Echo wave slot contribution")
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    custom.set_editor_property(
        "inputs",
        [custom_input(name) for name, _input_type in inputs],
    )
    custom.set_editor_property(
        "code",
        "\n".join(
            [
                "float elapsed = TimeValue - StartTime;",
                "float safeSpeed = max(Speed, 0.001);",
                "float reachTime = MaxRadius / safeSpeed;",
                "float effectiveDuration = min(max(Duration, 0.001), max(reachTime, 0.001));",
                "float active = step(0.0001, StartTime) * step(0.0, elapsed) * (1.0 - step(effectiveDuration, elapsed));",
                "float clampedElapsed = max(elapsed, 0.0);",
                "float radius = min(MaxRadius, clampedElapsed * Speed);",
                "float distToFront = abs(distance(WorldPosition, Origin.xyz) - radius);",
                "float band = saturate(1.0 - distToFront / max(Width, 1.0));",
                "float fade = saturate(1.0 - clampedElapsed / max(effectiveDuration, 0.01));",
                "return band * fade * WaveIntensity * active;",
            ]
        ),
    )

    for name, _input_type in inputs:
        if not unreal.MaterialEditingLibrary.connect_material_expressions(input_nodes[name], "", custom, name):
            raise RuntimeError(f"Failed to connect function input {name}")

    output = unreal.MaterialEditingLibrary.create_material_expression_in_function(
        material_function,
        unreal.MaterialExpressionFunctionOutput,
        180,
        0,
    )
    output.set_editor_property("output_name", "Wave")
    output.set_editor_property("sort_priority", 0)

    if not unreal.MaterialEditingLibrary.connect_material_expressions(custom, "", output, ""):
        raise RuntimeError("Failed to connect custom slot output")

    unreal.MaterialEditingLibrary.layout_material_function_expressions(material_function)
    unreal.MaterialEditingLibrary.update_material_function(material_function)
    save_asset_checked(FUNCTION_PATH)
    log(f"Updated {FUNCTION_PATH}")


def material_expression(material, expression_class, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(material, expression_class, x, y)


def collection_parameter(material, mpc, name, x, y):
    expression = material_expression(material, unreal.MaterialExpressionCollectionParameter, x, y)
    expression.set_editor_property("collection", mpc)
    expression.set_editor_property("parameter_name", name)
    return expression


def function_call(material, material_function, slot_index, x, y):
    expression = material_expression(material, unreal.MaterialExpressionMaterialFunctionCall, x, y)
    if not expression.set_material_function(material_function):
        raise RuntimeError(f"Failed to assign {FUNCTION_PATH} to function call {slot_index}")
    expression.set_editor_property("desc", f"Echo slot {slot_index}")
    return expression


def add_node(material, left, right, x, y):
    expression = material_expression(material, unreal.MaterialExpressionAdd, x, y)
    if not unreal.MaterialEditingLibrary.connect_material_expressions(left, "", expression, "A"):
        raise RuntimeError("Failed to connect Add.A")
    if not unreal.MaterialEditingLibrary.connect_material_expressions(right, "", expression, "B"):
        raise RuntimeError("Failed to connect Add.B")
    return expression


def multiply_node(material, left, right, x, y):
    expression = material_expression(material, unreal.MaterialExpressionMultiply, x, y)
    if not unreal.MaterialEditingLibrary.connect_material_expressions(left, "", expression, "A"):
        raise RuntimeError("Failed to connect Multiply.A")
    if not unreal.MaterialEditingLibrary.connect_material_expressions(right, "", expression, "B"):
        raise RuntimeError("Failed to connect Multiply.B")
    return expression


def build_16_slot_material(material, material_function, mpc):
    duplicate_backup_if_needed(MATERIAL_PATH, MATERIAL_BACKUP_PATH)
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

    set_optional_property(material, "material_domain", unreal.MaterialDomain.MD_SURFACE)
    set_optional_property(material, "blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    set_optional_property(material, "shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    set_optional_property(material, "use_material_attributes", False)

    world_position = material_expression(material, unreal.MaterialExpressionWorldPosition, -1600, -600)
    time_value = material_expression(material, unreal.MaterialExpressionTime, -1600, -460)

    slot_outputs = []
    for slot_index in range(SLOT_COUNT):
        row_y = -900 + slot_index * 180
        call = function_call(material, material_function, slot_index, -520, row_y + 40)
        parameter_nodes = {
            "Origin": collection_parameter(material, mpc, f"EchoOrigin{slot_index}", -1180, row_y - 100),
            "StartTime": collection_parameter(material, mpc, f"EchoStartTime{slot_index}", -1180, row_y - 60),
            "Speed": collection_parameter(material, mpc, f"EchoSpeed{slot_index}", -1180, row_y - 20),
            "Width": collection_parameter(material, mpc, f"EchoWidth{slot_index}", -1180, row_y + 20),
            "Intensity": collection_parameter(material, mpc, f"EchoIntensity{slot_index}", -1180, row_y + 60),
            "Duration": collection_parameter(material, mpc, f"EchoDuration{slot_index}", -1180, row_y + 100),
            "MaxRadius": collection_parameter(material, mpc, f"EchoMaxRadius{slot_index}", -1180, row_y + 140),
        }

        connections = [
            (world_position, "WorldPosition"),
            (time_value, "TimeValue"),
            (parameter_nodes["Origin"], "Origin"),
            (parameter_nodes["StartTime"], "StartTime"),
            (parameter_nodes["Speed"], "Speed"),
            (parameter_nodes["Width"], "Width"),
            (parameter_nodes["Intensity"], "WaveIntensity"),
            (parameter_nodes["Duration"], "Duration"),
            (parameter_nodes["MaxRadius"], "MaxRadius"),
        ]
        for source, input_name in connections:
            if not unreal.MaterialEditingLibrary.connect_material_expressions(source, "", call, input_name):
                raise RuntimeError(f"Failed to connect slot {slot_index} input {input_name}")

        slot_outputs.append(call)

    running_total = slot_outputs[0]
    for slot_index, slot_output in enumerate(slot_outputs[1:], start=1):
        running_total = add_node(material, running_total, slot_output, -80 + (slot_index % 4) * 140, -420 + slot_index * 55)

    color = material_expression(material, unreal.MaterialExpressionConstant3Vector, 720, -180)
    color.set_editor_property("constant", unreal.LinearColor(0.05, 0.75, 1.0, 1.0))

    emissive = multiply_node(material, color, running_total, 980, -60)

    if not unreal.MaterialEditingLibrary.connect_material_property(
        emissive,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    ):
        raise RuntimeError("Failed to connect emissive color")

    if not unreal.MaterialEditingLibrary.connect_material_property(
        emissive,
        "",
        unreal.MaterialProperty.MP_BASE_COLOR,
    ):
        log("BaseColor connection was not accepted; emissive output is still connected.")

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    save_asset_checked(MATERIAL_PATH)
    log(f"Updated {MATERIAL_PATH} with {SLOT_COUNT} echo wave function calls")


def main():
    mpc = load_asset_checked(MPC_PATH)
    material = load_asset_checked(MATERIAL_PATH)
    validate_mpc_parameters(mpc)

    material_function = get_or_create_material_function()
    build_echo_wave_slot_function(material_function)
    build_16_slot_material(material, material_function, mpc)


main()
