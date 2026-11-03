-- Bundled EasyFace native installer; ELF/icon live beside main.lua.
local lvgl = require("lvgl")
local ELF = "s4_calculator_5.4.2.elf"
local MODULE = "s4_native_calculator"
local ELF_SIZE = 11252
local root = lvgl.Object(nil, {
    w = lvgl.HOR_RES(), h = lvgl.VER_RES(), bg_color = 0x07111F,
    bg_opa = lvgl.OPA(100), border_width = 0,
})
root:clear_flag(lvgl.FLAG.SCROLLABLE)
root:add_flag(lvgl.FLAG.EVENT_BUBBLE)
local status = lvgl.Label(root, {
    text = "Native Calculator r6\nS4 Global 5.4.2\nTap START to install.",
    text_color = 0xBFD9FF, width = 300,
    text_font = lvgl.Font("NotoSansMerge-Regular", 20, "normal"),
    align = { type = lvgl.ALIGN.CENTER, x_ofs = 0, y_ofs = -50 },
})
local function show(text) status:set { text = tostring(text) } end
local function read(path)
    local f = io.open(path, "rb")
    if not f then return nil end
    local data = f:read("*a")
    f:close()
    return data
end
local function capture(command)
    local path = "/tmp/s4_native_result"
    os.remove(path)
    os.execute(command .. " > " .. path)
    return read(path) or ""
end
local function loaded(name)
    return capture("lsmod"):find(name, 1, true) ~= nil
end
local function copy_bundled(name, destination, size, elf)
    if type(SCRIPT_PATH) ~= "string" or SCRIPT_PATH == "" then
        show("SCRIPT_PATH is missing.") return false
    end
    local folder = SCRIPT_PATH
    if folder:sub(-1) ~= "/" then folder = folder .. "/" end
    local content = read(folder .. name)
    if not content or #content ~= size or (elf and content:sub(1,4) ~= "\127ELF") then
        show("Missing/wrong bundled file:\n" .. name .. "\nKeep it beside main.lua.")
        return false
    end
    local f = io.open(destination, "wb")
    if not f then show("Cannot write:\n" .. destination) return false end
    local written = f:write(content)
    local closed = f:close()
    if not written or not closed or read(destination) ~= content then
        show("File copy failed.") return false
    end
    return true
end
local function install()
    local fw = capture("getprop ro.build.version"):match("^%s*([%d.]+)")
    if fw ~= "5.4.2" then
        show("Unsupported firmware:\n" .. tostring(fw) .. "\nRequires S4 Global 5.4.2.") return
    end
    -- Firmware uses ro.product.name for the display name. ro.product.model
    -- can be absent or an internal value such as "ap". Missing properties
    -- are not a mismatch: the ELF checks exact firmware anchors on entry.
    local name = capture("getprop ro.product.name"):gsub("^%s+", ""):gsub("%s+$", "")
    if name ~= "" and name ~= "Xiaomi Watch S4" then
        show("Unsupported product:\n" .. name .. "\nRequires Xiaomi Watch S4.") return
    end
    if lvgl.HOR_RES() ~= 466 or lvgl.VER_RES() ~= 466 then
        show("Unsupported display.\nRequires 466 x 466.") return
    end
    if loaded(MODULE) then show("Already loaded this boot.\nReboot before replacing it.") return end
    if not copy_bundled("s4_calc_logo_112.png", "/data/s4_calc_logo_112.png", 19531, false) then return end
    if not copy_bundled(ELF, "/data/" .. ELF, ELF_SIZE, true) then return end
    local log = "/tmp/s4_native_insmod"
    os.remove(log)
    local ok = os.execute("insmod /data/" .. ELF .. " " .. MODULE .. " > " .. log)
    if (ok ~= true and ok ~= 0) or not loaded(MODULE) then
        show("Native load failed:\n" .. (read(log) or tostring(ok)):sub(1,180)) return
    end
    show("Finishing installation...\nCalculator will appear\nin your apps.")
    return true
end
local button = lvgl.Object(root, {
    w = 240, h = 68, bg_color = 0x14508A, bg_opa = lvgl.OPA(100), radius = 16,
    align = { type = lvgl.ALIGN.CENTER, x_ofs = 0, y_ofs = 105 },
})
button:clear_flag(lvgl.FLAG.SCROLLABLE)
button:add_flag(lvgl.FLAG.EVENT_BUBBLE)
button:add_flag(lvgl.FLAG.CLICKABLE)
lvgl.Label(button, {
    text = "START", text_color = 0xFFFFFF, align = lvgl.ALIGN.CENTER,
    text_font = lvgl.Font("NotoSansMerge-Regular", 28, "normal"),
})
local busy = false
button:onClicked(function()
    if busy then return end
    busy = true
    local ok, result = pcall(install)
    busy = ok and result == true
    if not ok then show("Error:\n" .. tostring(result)) end
end)
