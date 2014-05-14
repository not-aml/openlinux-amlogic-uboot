m1_8726m_mid_config: unconfig
	$(MKCONFIG) $(@:_config=)  arm aml_meson 8726m_mid_refb10 amlogic m1
8726m_mid_refb10_config_help:
	@echo =======================================================================
	@echo The mark in board is "MID"
	@echo config command: \"make $(@:%_help=%)\"
help:8726m_mid_refb10_config_help
