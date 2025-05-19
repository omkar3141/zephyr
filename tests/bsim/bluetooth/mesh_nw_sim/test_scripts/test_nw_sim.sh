#!/usr/bin/env bash
# Copyright 2025 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../_mesh_test.sh

# Test scenario:
overlay=overlay_psa_conf_overlay_pst_conf_overlay_ss_conf
RunTest mesh_network_sim_test \
                check_node_device check_node_device

