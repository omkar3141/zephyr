#!/usr/bin/env bash
# Copyright 2026 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# This test verifies that application key deletion is synchronized between mesh and cdb.
#
# Test procedure:
# 1. Provisions DUT.
# 2. Adds the application key and checks that it is created in cdb.
# 3. Deletes the added application key.
# 4. Checks that there is no added application key in cdb.

RunTest mesh_cdb_appkey_delete cdb_sync_appkey_delete
