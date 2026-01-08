#!/usr/bin/env bash
# Copyright 2026 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# This test verifies that multiple application keys are synchronized between
# mesh and cdb during the key refresh procedure.
#
# Test procedure:
# 1. Provisions DUT.
# 2. Adds two application keys bound to the primary subnet and checks that
#    they are created in cdb.
# 3. Updates primary network key and application keys to trigger the key refresh procedure.
# 4. Checks that the application keys are updated in cdb.
# 5. Puts the key refresh procedure to phase 2 and then to phase 3 and checks that
#    cdb application key instances and corresponding application keys in core are the same.

RunTest mesh_cdb_multi_appkey_kr cdb_sync_multiple_appkeys_kr
