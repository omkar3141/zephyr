#!/usr/bin/env bash
# Copyright 2021 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# RunTest mesh_scheduler_model_initialization scheduler_model_server_check_init scheduler_model_client_check_init

# SKIP=(scheduler_model_server_check_init)
RunTest mesh_scheduler_model_initialization scheduler_model_server_check_init

