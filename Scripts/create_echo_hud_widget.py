import unreal


ASSET_PATH = "/Game/UI/WBP_EchoHUD"
ASSET_NAME = "WBP_EchoHUD"
PACKAGE_PATH = "/Game/UI"


def log(message):
    unreal.log(f"[CreateEchoHUDWidget] {message}")


def set_if_possible(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
        return True
    except Exception:
        return False


def make_text(widget, text, size=24, color=None, justification=None):
    try:
        widget.set_text(unreal.Text(text))
    except Exception:
        pass

    font = None
    try:
        font = widget.get_editor_property("font")
        font.size = size
        widget.set_editor_property("font", font)
    except Exception:
        pass

    if color is not None:
        try:
            widget.set_color_and_opacity(unreal.SlateColor(color))
        except Exception:
            set_if_possible(widget, "color_and_opacity", unreal.SlateColor(color))

    if justification is not None:
        set_if_possible(widget, "justification", justification)


def add_to_canvas(canvas, widget, anchors, position, size, alignment=(0.0, 0.0), auto_size=False):
    slot = canvas.add_child_to_canvas(widget)
    try:
        slot.set_anchors(unreal.Anchors(*anchors))
    except Exception:
        set_if_possible(slot, "anchors", unreal.Anchors(*anchors))

    try:
        slot.set_position(unreal.Vector2D(*position))
    except Exception:
        set_if_possible(slot, "position", unreal.Vector2D(*position))

    try:
        slot.set_size(unreal.Vector2D(*size))
    except Exception:
        set_if_possible(slot, "size", unreal.Vector2D(*size))

    try:
        slot.set_alignment(unreal.Vector2D(*alignment))
    except Exception:
        set_if_possible(slot, "alignment", unreal.Vector2D(*alignment))

    try:
        slot.set_auto_size(auto_size)
    except Exception:
        set_if_possible(slot, "auto_size", auto_size)

    return slot


def create_or_load_widget_blueprint():
    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        log(f"Updating existing asset: {ASSET_PATH}")
        return unreal.EditorAssetLibrary.load_asset(ASSET_PATH)

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", unreal.EchoHUDWidget)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(ASSET_NAME, PACKAGE_PATH, unreal.WidgetBlueprint, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {ASSET_PATH}")

    log(f"Created asset: {ASSET_PATH}")
    return asset


def get_widget_tree(widget_blueprint):
    try:
        return widget_blueprint.get_editor_property("widget_tree")
    except Exception:
        tree = getattr(widget_blueprint, "widget_tree", None)
        if tree is None:
            raise RuntimeError("Widget blueprint has no accessible widget_tree")
        return tree


def clear_root(widget_tree):
    try:
        widget_tree.set_editor_property("root_widget", None)
    except Exception:
        pass


def construct(widget_tree, cls, name):
    return widget_tree.construct_widget(cls, name)


def build_layout(widget_blueprint):
    widget_tree = get_widget_tree(widget_blueprint)
    clear_root(widget_tree)

    root = construct(widget_tree, unreal.CanvasPanel, "Root_Canvas")
    widget_tree.set_editor_property("root_widget", root)

    cyan = unreal.LinearColor(0.55, 0.95, 1.0, 1.0)
    white = unreal.LinearColor(0.92, 0.96, 1.0, 1.0)
    state = unreal.LinearColor(1.0, 1.0, 1.0, 1.0)

    text_score = construct(widget_tree, unreal.TextBlock, "Text_Score")
    make_text(text_score, "K 0  D 0  Target 10", 24, cyan)
    add_to_canvas(root, text_score, (0.0, 0.0, 0.0, 0.0), (260.0, 26.0), (360.0, 40.0), (0.0, 0.0), False)

    text_health = construct(widget_tree, unreal.TextBlock, "Text_Health")
    make_text(text_health, "HP 100 / 100", 24, white)
    add_to_canvas(root, text_health, (0.0, 1.0, 0.0, 1.0), (28.0, -92.0), (260.0, 32.0), (0.0, 1.0), False)

    progress_health = construct(widget_tree, unreal.ProgressBar, "Progress_Health")
    try:
        progress_health.set_percent(1.0)
    except Exception:
        set_if_possible(progress_health, "percent", 1.0)
    set_if_possible(progress_health, "fill_color_and_opacity", unreal.LinearColor(0.0, 0.85, 0.78, 1.0))
    add_to_canvas(root, progress_health, (0.0, 1.0, 0.0, 1.0), (28.0, -52.0), (260.0, 18.0), (0.0, 1.0), False)

    text_weapon = construct(widget_tree, unreal.TextBlock, "Text_Weapon")
    make_text(text_weapon, "1 Standard", 26, cyan)
    add_to_canvas(root, text_weapon, (1.0, 1.0, 1.0, 1.0), (-280.0, -52.0), (250.0, 38.0), (1.0, 1.0), False)

    text_state = construct(widget_tree, unreal.TextBlock, "Text_State")
    make_text(text_state, "", 48, state, unreal.TextJustify.CENTER)
    text_state.set_visibility(unreal.SlateVisibility.HIDDEN)
    add_to_canvas(root, text_state, (0.5, 0.5, 0.5, 0.5), (-320.0, -120.0), (640.0, 180.0), (0.0, 0.0), False)

    widget_blueprint.set_editor_property("display_label", "Echo HUD")
    log("Built HUD widget tree")


def compile_and_save(widget_blueprint):
    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    except Exception as exc:
        log(f"Compile via BlueprintEditorLibrary skipped/failed: {exc}")

    unreal.EditorAssetLibrary.save_asset(ASSET_PATH, only_if_is_dirty=False)
    log(f"Saved {ASSET_PATH}")


def main():
    widget_blueprint = create_or_load_widget_blueprint()
    try:
        build_layout(widget_blueprint)
    except Exception as exc:
        log(f"Skipped automated widget tree layout: {exc}")
        log("The asset was created with EchoHUDWidget as parent. Add Text_Health, Progress_Health, Text_Score, Text_Weapon, and Text_State in UMG Designer.")
    compile_and_save(widget_blueprint)


if __name__ == "__main__":
    main()
