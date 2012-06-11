#!/bin/sh
swig -java -Wall -outdir Glue/Android/org/dynamo/ -package org.dynamo -module jni dynamo.i
