#!/bin/sh
sed 's/MB file/KB block/' fileContention  | awk '{ print $1" Cost of a "$5" read = "($8 / 1024) / 3300000, "ms" } '
