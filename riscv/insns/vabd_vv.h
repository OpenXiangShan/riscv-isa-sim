// vabd.vv vd, vs2, vs1, vm

require_zvabd;

#define VABD_VV(SEW) \
  type_usew_t<SEW>::type &vd = P.VU.elt<type_usew_t<SEW>::type>(rd_num, i, true); \
  type_sew_t<SEW>::type vs1 = P.VU.elt<type_sew_t<SEW>::type>(rs1_num, i); \
  type_sew_t<SEW>::type vs2 = P.VU.elt<type_sew_t<SEW>::type>(rs2_num, i); \
  vd = DO_ABD(vs2, vs1);

VI_CHECK_SSS(true)
VI_LOOP_BASE
// Use unsigned element access only to write the SEW-bit truncated result.
if (sew == e8) {
  VABD_VV(e8);
} else if (sew == e16) {
  VABD_VV(e16);
} else if (sew == e32) {
  VABD_VV(e32);
} else if (sew == e64) {
  VABD_VV(e64);
}
VI_LOOP_END

#undef VABD_VV
