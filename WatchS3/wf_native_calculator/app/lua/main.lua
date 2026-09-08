-- m0tral (c) 2026-09-04
-- native app installer

local lvgl = require("lvgl")

local LOGO_NAME   = "calc_logo_112.png"

-- Firmware -> app ELF. Add a row whenever a new firmware build is supported;
local FW_TO_APP = {
    ["4.8.0"] = "watchs3_app_4.8.0.elf",
    ["2.6.0"] = "watchs3_app_2.6.0.elf",
}

local function detect_fw_version()
    -- Probe the device once and cache the result on /tmp.
    os.execute("getprop ro.build.version > /tmp/device_version")
    local f = io.open("/tmp/device_version", "r")
    if not f then return nil end
    local raw = f:read("*l") or ""
    f:close()
    -- Pull just the leading dotted/digit/underscore token ("4.8.0"
    -- or "4_8_0"); the trailing "release-keys" etc. is dropped so the
    -- lookup table can use the canonical dotted form.
    return (raw:match("^%s*([%d._]+)") or ""):gsub("%s+", "")
end

local fw     = detect_fw_version()
local APP_NAME = FW_TO_APP[fw]
local LOAD_ERROR = nil
if not APP_NAME then
    LOAD_ERROR = "unsupported firmware: " .. tostring(fw)
                 .. "\nadd a FW_TO_APP entry for this build"
    APP_NAME = ""
end

local MODULE_NAME = "watchs3_app"

local APP_SRC     = SCRIPT_PATH .. APP_NAME
local APP_DST     = "/data/watchs3_com.m0tral.calculator.elf"

local LOGO_SRC    = SCRIPT_PATH .. LOGO_NAME
local LOGO_DST    = "/data/" .. LOGO_NAME

local rootbase = lvgl.Object(nil, {
    w = lvgl.HOR_RES(), h = lvgl.VER_RES(), bg_color = 0x07111F,
    bg_opa = lvgl.OPA(100), border_width = 0,
})
rootbase:clear_flag(lvgl.FLAG.SCROLLABLE)
rootbase:add_flag(lvgl.FLAG.EVENT_BUBBLE)
local root = lvgl.Object(rootbase, {
    w = lvgl.HOR_RES(), h = lvgl.VER_RES(), bg_color = 0x07111F,
    bg_opa = lvgl.OPA(100),
    border_width = 0, pad_all = 0, align = lvgl.ALIGN.CENTER,
})
root:clear_flag(lvgl.FLAG.SCROLLABLE)
root:add_flag(lvgl.FLAG.EVENT_BUBBLE)

local status
local function set_status(text, color)
    if status then status:set { text = tostring(text), text_color = color or 0xBFD9FF } end
end

local function make_button(text, y, color, on_clicked)
    local button = lvgl.Object(root, {
        w = 320, h = 80, bg_color = color, bg_opa = lvgl.OPA(100),
        radius = 16,
        align = { type = lvgl.ALIGN.CENTER, x_ofs = 0, y_ofs = y },
    })
    button:clear_flag(lvgl.FLAG.SCROLLABLE)
    button:add_flag(lvgl.FLAG.EVENT_BUBBLE)
    button:add_flag(lvgl.FLAG.CLICKABLE)
    lvgl.Label(button, {
        text = text, text_color = 0xFFFFFF, align = lvgl.ALIGN.CENTER,
        text_font = lvgl.Font("NotoSansMerge-Regular", 28, "normal")
    })
    button:onClicked(function()
        local ok, message = pcall(on_clicked)
        if not ok then set_status("Error: " .. tostring(message), 0xFF9A9A) end
    end)
    return button
end

local run_button = make_button("START", -20,
    LOAD_ERROR and 0x555555 or 0x14508A,
    function()
        if LOAD_ERROR then
            set_status(LOAD_ERROR, 0xFF9A9A)
            return
        end
        set_status("cp " .. APP_SRC .. " " .. APP_DST)
        local cp_ok = os.execute("cp " .. APP_SRC .. " " .. APP_DST)
        if cp_ok ~= true and cp_ok ~= 0 then
            set_status("cp failed (code=" .. tostring(cp_ok) .. ")", 0xFF9A9A)
            return
        end
        set_status("app: " .. APP_DST .. " " .. MODULE_NAME)
        os.execute("cp " .. LOGO_SRC .." ".. LOGO_DST);
        os.execute("rmmod " .. MODULE_NAME)
        local ok = os.execute("insmod " .. APP_DST .. " " .. MODULE_NAME)
        if ok == true or ok == 0 then
            set_status("app ok", 0x8FF0A4)
        else
            set_status("app failed (code=" .. tostring(ok) .. ")", 0xFF9A9A)
        end
    end)

if LOAD_ERROR then
    run_button:clear_flag(lvgl.FLAG.CLICKABLE)
end

status = lvgl.Label(root, {
    text = LOAD_ERROR
        or ("Ready\nfw=" .. (fw or "?") .. "\nmodule=" .. MODULE_NAME),
    text_color = LOAD_ERROR and 0xFF9A9A or 0xBFD9FF,
    width = 300, height = 160,
    text_font = lvgl.Font("NotoSansMerge-Regular", 20, "normal"),
    align = { type = lvgl.ALIGN.CENTER, x_ofs = 0, y_ofs = 112 },
})
