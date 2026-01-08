#!/usr/bin/env bash
# Copyright 2026 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# This test verifies that mesh subnet and cdb implementations keep network key in sync
# during key refresh procedure.
#
# Test procedure:
# 1. Provisions DUT and checks that the primary subnet is created.
# 2. Updates primary network key to start key refresh procedure.
# 3. Checks that cdb subnet key refresh phase and key are updated accordingly.
# 4. Put the key refresh procedure to phase 2 and checks cdb instance phase.
# 5. Completes key refresh procedure and checks that cdb and primary network keys
#    are the same key.

RunTest mesh_cdb_subnet_kr cdb_sync_subnet_kr
