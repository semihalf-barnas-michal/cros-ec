#!/bin/bash
#
# Copyright 2014 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Verify there is no CPRINTS("....\n", ...) statements added to the code.
upstream_branch="$(git rev-parse --abbrev-ref --symbolic-full-name @{u} \
    2>/dev/null)"
if [[ -z ${upstream_branch} ]]; then
  echo "Current branch does not have an upstream branch" >&2
  exit 1
fi
# This will print the offending CPRINTS invocations, if any, and the names of
# the files they are in.
if git diff --no-ext-diff "${upstream_branch}" HEAD |
    grep -e '^+\(.*CPRINTS(.*\\n"\|++\)' |
    grep CPRINTS -B1 >&2 ; then
  echo "error: CPRINTS strings should not include newline characters" >&2
  exit 1
fi
