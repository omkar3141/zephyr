#!/usr/bin/env bash
# Copyright 2021 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

$(dirname "${BASH_SOURCE[0]}")/pst_provisioning_save.sh

$(dirname "${BASH_SOURCE[0]}")/pst_provisioning_load.sh

