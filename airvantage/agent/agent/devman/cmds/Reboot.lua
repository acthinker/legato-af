--------------------------------------------------------------------------------
-- Copyright (c) 2012 Sierra Wireless and others.
-- All rights reserved. This program and the accompanying materials
-- are made available under the terms of the Eclipse Public License v1.0
-- and Eclipse Distribution License v1.0 which accompany this distribution.
--
-- The Eclipse Public License is available at
--   http://www.eclipse.org/legal/epl-v10.html
-- The Eclipse Distribution License is available at
--   http://www.eclipse.org/org/documents/edl-v10.php
--
-- Contributors:
--     Sierra Wireless - initial API and implementation
-------------------------------------------------------------------------------
local system = require 'agent.system'

function Reboot(asset, data, path, ticket)
    local status
    if ticket and ticket ~= 0 then
        require'racon'.acknowledge(ticket, 0, "Reboot command succeeded", "on_boot", true) --ack to server after rebooting device
        status = "async"
    else status = "ok" end

    system.reboot(nil, "request from server")
    return status
end

return Reboot
