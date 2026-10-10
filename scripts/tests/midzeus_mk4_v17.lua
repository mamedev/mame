-- Compiled MAME v17 integration test; expected channels are 5 and 30.
-- Disposable compiled-MAME test. All fixture geometry and tables are synthetic.
-- These results test the candidate implementation, not physical Zeus hardware.
local out = assert(os.getenv('ZEUS_TEST_OUT'))
local space = manager.machine.devices[':maincpu'].spaces['program']
local share = assert(manager.machine.memory.shares[':zeusbase'])
local frames = 0
local function wr(r, v) space:write_u32(0x880000 + r, v) end
local function rd(r) return space:read_u32(0x880000 + r) end
local function fifo(words) for _, v in ipairs(words) do wr(0xe0, v) end end
local function upload(expanded, words)
    wr(0xb6, 0x02f00000)
    for i = 1, #words, 2 do
        wr(0xb4, expanded + (i - 1) // 2)
        wr(0xb0, words[i])
        wr(0xb2, words[i + 1])
    end
    wr(0xb6, 0)
end
local function table_words(bit)
    local words = {}
    for i = 0, 31 do
        local word = 0
        for byte = 0, 3 do
            local index = i * 4 + byte
            local value = ((index >> bit) & 1) ~= 0 and 192 or 32
            word = word | (value << (8 * byte))
        end
        words[#words + 1] = word
    end
    return words
end
local function packed_xy(x, y) return (x & 0xffff) | ((y & 0xffff) << 16) end
local function read_pixel(x, y)
    local block = (y << 8) + (x >> 1)
    wr(0xb6, 0x80010000)
    wr(0xb4, ((block >> 9) << 16) | (block & 0x1ff))
    local word = rd(0xb0)
    wr(0xb6, 0)
    return (word >> (16 * (x & 1))) & 0x7fff
end
local function draw(light_x, normals, skip_light)
    upload(0x00200000, {
        0x25810000, 0x000c0000,
        packed_xy(-512, -512), 8192,
        packed_xy(512, -512), 8192,
        packed_xy(512, 512), 8192,
        packed_xy(-512, 512), 8192,
        normals[1], normals[2], normals[3], normals[4]
    })
    if not skip_light then fifo({0x23000000, light_x & 0xffff}) end
    fifo({0x01008000, 0x06020000})
    fifo({0x13000000})
end
ZEUS_REGISTER_TEST_SUB = emu.add_machine_frame_notifier(function()
    frames = frames + 1
    if frames ~= 2000 then return end
    local log = assert(io.open(out .. '/register-test.tsv', 'w'))
    log:write('case\texpected_rgb555\tactual_rgb555\tresult\n')
    local ok, err = pcall(function()
        assert((share:read_u32(0x80 * 4) & 0x20000) ~= 0, '32-bit mode missing')
        -- Read the current buffer once to drain asynchronous rendering.
        read_pixel(200, 128)
        wr(0x84, 0)
        wr(0x00, 0x7fff)
        wr(0x04, 0x0800) -- no depth test or write; no texture is used
        wr(0x5c, 0x4b23cb00) -- existing intensity modulation path
        fifo({0x1c000000, 0x00004000, 0x00004000, 0x40000000, 0, 0, 0, 0})
        local rows = {
            {'negative-bit0', -25, 0, 0x14a5},
            {'negative-bit1', -25, 1, 0x7bde},
            {'overflow-bit0', 1024, 0, 0x7bde}
        }
        for _, row in ipairs(rows) do
            upload(0x00100000, table_words(row[3]))
            fifo({0x0102c0f0, 0x0f010000})
            draw(row[2], {256, 256, 256, 256})
            local actual = read_pixel(200, 128)
            log:write(string.format('%s\t%04x\t%04x\t%s\n', row[1], row[4], actual, actual == row[4] and 'PASS' or 'FAIL'))
            assert(actual == row[4], row[1] .. ' mismatch')
        end
        draw(-25, {0, 0, 0, 0})
        local actual = read_pixel(200, 128)
        assert(actual == 0x7fff, 'Zero-normal fallback mismatch')
        log:write(string.format('zero-normal\t7fff\t%04x\tPASS\n', actual))
        fifo({0x0102c0f0, 0x0e010000})
        draw(-25, {256, 256, 256, 256})
        actual = read_pixel(200, 128)
        assert(actual == 0x7fff, 'Short-table fallback mismatch')
        log:write(string.format('short-table\t7fff\t%04x\tPASS\n', actual))
        -- A response change at the same table address must be read on the next draw.
        -- On this vector bit0 is low and bit1 is high; no pointer rebind is issued.
        fifo({0x0102c0f0, 0x0f010000})
        upload(0x00100000, table_words(0))
        draw(-25, {256, 256, 256, 256})
        assert(read_pixel(200, 128) == 0x14a5, 'First table version mismatch')
        upload(0x00100000, table_words(1))
        draw(-25, {256, 256, 256, 256})
        actual = read_pixel(200, 128)
        assert(actual == 0x7bde, 'Updated table not visible')
        log:write(string.format('table-rewrite\t7bde\t%04x\tPASS\n', actual))
        local valid_item, table_item
        for name, idx in pairs(manager.machine.devices[':'].items) do
            if name:find('m_zeus_light_valid', 1, true) then valid_item = emu.item(idx) end
            if name:find('m_zeus_unkbase', 1, true) then table_item = emu.item(idx) end
        end
        assert(valid_item and table_item, 'New lighting state not registered')
        assert(valid_item:read(0) == 1 and table_item:read(0) == 0x0f010000)
        local saved = manager.machine:buffer_save()
        assert(type(saved) == 'string' and #saved > 0)
        valid_item:write(0, 0)
        draw(-25, {256, 256, 256, 256}, true)
        actual = read_pixel(200, 128)
        assert(actual == 0x7fff, 'Missing-light fallback mismatch')
        log:write(string.format('missing-light\t7fff\t%04x\tPASS\n', actual))
        table_item:write(0, 0)
        upload(0x00100000, table_words(0))
        assert(manager.machine:buffer_load(saved), 'State restore failed')
        saved = nil -- no game-state buffer is exported
        assert(valid_item:read(0) == 1 and table_item:read(0) == 0x0f010000, 'Lighting fields not restored')
        draw(-25, {256, 256, 256, 256}, true)
        actual = read_pixel(200, 128)
        assert(actual == 0x7bde, 'Restored lighting response mismatch')
        log:write(string.format('save-load-light-table\t7bde\t%04x\tPASS\n', actual))
    end)
    if not ok then log:write('ERROR\t' .. tostring(err) .. '\n') end
    log:close()
    local terminal = assert(io.open(out .. '/register-test.status', 'w'))
    terminal:write(ok and 'PASS\n' or 'FAIL\n'); terminal:close()
    manager.machine:exit()
end)
