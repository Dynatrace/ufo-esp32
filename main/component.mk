#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

#COMPONENT_EMBED_TXTFILES := wpa2_ca.pem
COMPONENT_EMBED_TXTFILES := cacert.pem
COMPONENT_EMBED_TXTFILES += prvtkey.pem