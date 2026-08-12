# Test name - needs to match the directory name
TESTS += ntt-kyber

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
NTT_KYBER_PLATFORMS += cross-rv64im
NTT_KYBER_PLATFORMS += cross-rvv

# C sources required for this test
NTT_KYBER_SOURCES += main.c
NTT_KYBER_SOURCES += kyber-ref.c

# Assembly sources required for this test
NTT_KYBER_ASM_DIR = ../../asm/manual/ntt_kyber

# --- RV64IM: scalar forward NTT, single-issue (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_singleissue_plant_rv64im.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_singleissue_plant_rv64im_opt_c908.s

# --- RV64IM: scalar forward NTT, dual-issue (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_dualissue_plant_rv64im.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_dualissue_plant_rv64im_opt_c908.s

# --- RV64IM: scalar forward NTT, dual-issue l=32 (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_dualissue_l32_plant_rv64im.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_dualissue_l32_plant_rv64im_opt_c908.s

# --- RV64IM: scalar inverse NTT, single-issue (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_singleissue_plant_rv64im.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_singleissue_plant_rv64im_opt_c908.s

# --- RV64IM: scalar inverse NTT, dual-issue (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_dualissue_plant_rv64im.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_dualissue_plant_rv64im_opt_c908.s

# --- RVV VLEN=128: forward NTT (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_rvv_vlen128.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_rvv_vlen128_opt_c908.s

# --- RVV VLEN=128: inverse NTT (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_rvv_vlen128.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/intt_kyber_rvv_vlen128_opt_c908.s

# --- RVV VLEN=128: NTT <-> normal coefficient order (naive + optimized) ---
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/kyber_normal2ntt_order_rvv_vlen128.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/kyber_ntt2normal_order_rvv_vlen128.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/kyber_normal2ntt_order_rvv_vlen128_opt_c908.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/kyber_ntt2normal_order_rvv_vlen128_opt_c908.s
