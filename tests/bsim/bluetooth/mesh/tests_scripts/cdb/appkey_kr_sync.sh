#!/usr/bin/env bash
# Copyright 2026 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# This test verifies that mesh app key and cdb implementations keep application key in sync
# during key refresh procedure.
#
# Test procedure:
# 1. Provisions DUT, adds application key over config model and checks it is created in cdb.
# 2. Updates primary network and bound application keys to start key refresh procedure
#    on both network and application keys.
# 3. Checks that application key is updated accordingly.
# 4. Puts the key refresh procedure to phase 2 and then to phase 3 and checks that
#    cdb application key instance and corresponding application key in core are the same key.

RunTest mesh_cdb_appkey_kr cdb_sync_appkey_kr
