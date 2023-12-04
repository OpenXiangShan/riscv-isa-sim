// vssubu.vx vd, vs2, rs1
VI_CHECK_SSS(false);
VI_LOOP_BASE(1)
bool sat = false;

if (0 == P.VU.vta && i >= vl) { \
  continue; \
} \
if ((true == skip && 1 == P.VU.vma && i < vl) || (1 == P.VU.vta && i >= vl)) \
  mata_action = 2; \
else \
  mata_action = 1; \
switch (sew) {
case e8: {
  VX_U_PARAMS(e8);
  if (1 == mata_action) \
    vd = sat_subu<uint8_t>(vs2, rs1, sat);
  else \
    vd = vector_agnostic(vd); \
  break;
}
case e16: {
  VX_U_PARAMS(e16);
  if (1 == mata_action) \
    vd = sat_subu<uint16_t>(vs2, rs1, sat);
  else \
    vd = vector_agnostic(vd); \
  break;
}
case e32: {
  VX_U_PARAMS(e32);
  if (1 == mata_action) \
    vd = sat_subu<uint32_t>(vs2, rs1, sat);
  else \
    vd = vector_agnostic(vd); \
  break;
}
default: {
  VX_U_PARAMS(e64);
  if (1 == mata_action) \
    vd = sat_subu<uint64_t>(vs2, rs1, sat);
  else \
    vd = vector_agnostic(vd); \
  break;
}
}
P_SET_OV(sat);
VI_LOOP_END
