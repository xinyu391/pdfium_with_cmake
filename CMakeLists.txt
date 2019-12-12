cmake_minimum_required(VERSION 3.0)

project(pdfium)

set(CMAKE_BUILD_TYPE Release)
add_definitions(-O2)
add_definitions(-std=c++11)

# build agg

aux_source_directory(third_party/agg23 AGG_SRC)

add_definitions(-DDEFINE_PS_TABLES_DATA -DUSE_SYSTEM_ZLIB)
# -DUSE_SYSTEM_LIBJPEG
add_definitions(-DUSE_LIBJPEG_TURBO)
# icu i18n
add_definitions(-DU_I18N_IMPLEMENTATION -DU_COMMON_IMPLEMENTATION)

include_directories(third_party)
include_directories(.)

include_directories(third_party/freetype/src/include)

# libjpeg-turbo arm ubuntu need change to ASM
enable_language(ASM_NASM)

include_directories(third_party/libjpeg_turbo/)

if(CMAKE_ASM_COMPILER)
  add_definitions(-DWITH_SIMD)
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")

    include_directories(third_party/libjpeg_turbo/simd/nasm/)
    include_directories(third_party/libjpeg_turbo/simd/x86_64)

    set(CMAKE_ASM_NASM_FLAGS
        "${CMAKE_ASM_NASM_FLAGS} -I\"third_party/libjpeg_turbo/simd/nasm/\" -I\"${CMAKE_CURRENT_SOURCE_DIR}/${CPU_TYPE}/\""
    )

    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -DELF")
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -D__x86_64__")

    if(CMAKE_POSITION_INDEPENDENT_CODE OR ENABLE_SHARED)
      set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -DPIC")
    endif()

    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)
    set(EFFECTIVE_ASM_NASM_FLAGS
        "${CMAKE_ASM_NASM_FLAGS} ${CMAKE_ASM_NASM_FLAGS_${CMAKE_BUILD_TYPE_UC}}"
    )

    set(GREP grep)
    add_custom_target(
      jsimdcfg
      COMMAND
        ${CMAKE_C_COMPILER} -E -I${CMAKE_BINARY_DIR}
        -I${CMAKE_CURRENT_BINARY_DIR}
        -I${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjpeg_turbo/simd
        third_party/libjpeg_turbo/simd/nasm/jsimdcfg.inc.h | ${GREP} -E
        '^[\;%]|^\ %' | sed 's%_cpp_protection_%%' | sed 's@% define@%define@g'
        >${CMAKE_CURRENT_SOURCE_DIR}/nasm/jsimdcfg.inc)

    file(GLOB INC_FILES third_party/libjpeg_turbo/simd/nasm/*.inc)
    set(JPEG_SRC
        third_party/libjpeg_turbo/simd/x86_64/jccolor-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jccolor-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jcgray-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jcgray-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jchuff-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jcphuff-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jcsample-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jcsample-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdcolor-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdcolor-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdmerge-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdmerge-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdsample-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jdsample-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jfdctflt-sse.asm
        third_party/libjpeg_turbo/simd/x86_64/jfdctfst-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jfdctint-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jfdctint-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jidctflt-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jidctfst-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jidctint-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jidctint-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jidctred-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jquantf-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jquanti-avx2.asm
        third_party/libjpeg_turbo/simd/x86_64/jquanti-sse2.asm
        third_party/libjpeg_turbo/simd/x86_64/jsimdcpu.asm
        third_party/libjpeg_turbo/simd/x86_64/jsimd.c)

    foreach(file ${JPEG_SRC})
      set(OBJECT_DEPENDS "")
      if(${file} MATCHES jccolor)
        string(REGEX REPLACE "jccolor" "jccolext" DEPFILE ${file})
        set(OBJECT_DEPENDS
            ${OBJECT_DEPENDS}
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjpeg_turbo/simd/${DEPFILE}
        )
      endif()
      if(${file} MATCHES jcgray)
        string(REGEX REPLACE "jcgray" "jcgryext" DEPFILE ${file})
        set(OBJECT_DEPENDS
            ${OBJECT_DEPENDS}
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjpeg_turbo/simd/${DEPFILE}
        )
      endif()
      if(${file} MATCHES jdcolor)
        string(REGEX REPLACE "jdcolor" "jdcolext" DEPFILE ${file})
        set(OBJECT_DEPENDS
            ${OBJECT_DEPENDS}
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjpeg_turbo/simd/${DEPFILE}
        )
      endif()
      if(${file} MATCHES jdmerge)
        string(REGEX REPLACE "jdmerge" "jdmrgext" DEPFILE ${file})
        set(OBJECT_DEPENDS
            ${OBJECT_DEPENDS}
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libjpeg_turbo/simd/${DEPFILE}
        )
      endif()
      set(OBJECT_DEPENDS ${OBJECT_DEPENDS} ${INC_FILES})

      set_source_files_properties(${file} PROPERTIES OBJECT_DEPENDS
                                  "${OBJECT_DEPENDS}")
    endforeach()

  elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(JPEG_SRC
        third_party/libjpeg_turbo/simd/arm/arm64/jsimd.c
        third_party/libjpeg_turbo/simd/arm/arm64/jsimd_neon.S
        third_party/libjpeg_turbo/simd/arm/common/jdsample-neon.c)
  endif()
else()
  set(JPEG_SRC third_party/libjpeg_turbo/jsimd_none.c)
endif()

list(
  APPEND JPEG_SRC
         third_party/libjpeg_turbo/jcapimin.c
         third_party/libjpeg_turbo/jcapistd.c
         third_party/libjpeg_turbo/jccoefct.c
         third_party/libjpeg_turbo/jccolor.c
         third_party/libjpeg_turbo/jcdctmgr.c
         third_party/libjpeg_turbo/jchuff.c
         third_party/libjpeg_turbo/jchuff.h
         third_party/libjpeg_turbo/jcicc.c
         third_party/libjpeg_turbo/jcinit.c
         third_party/libjpeg_turbo/jcmainct.c
         third_party/libjpeg_turbo/jcmarker.c
         third_party/libjpeg_turbo/jcmaster.c
         third_party/libjpeg_turbo/jcomapi.c
         third_party/libjpeg_turbo/jcparam.c
         third_party/libjpeg_turbo/jcphuff.c
         third_party/libjpeg_turbo/jcprepct.c
         third_party/libjpeg_turbo/jcsample.c
         third_party/libjpeg_turbo/jctrans.c
         third_party/libjpeg_turbo/jdapimin.c
         third_party/libjpeg_turbo/jdapistd.c
         third_party/libjpeg_turbo/jdatadst.c
         third_party/libjpeg_turbo/jdatasrc.c
         third_party/libjpeg_turbo/jdcoefct.c
         third_party/libjpeg_turbo/jdcolor.c
         third_party/libjpeg_turbo/jddctmgr.c
         third_party/libjpeg_turbo/jdhuff.c
         third_party/libjpeg_turbo/jdhuff.h
         third_party/libjpeg_turbo/jdicc.c
         third_party/libjpeg_turbo/jdinput.c
         third_party/libjpeg_turbo/jdmainct.c
         third_party/libjpeg_turbo/jdmarker.c
         third_party/libjpeg_turbo/jdmaster.c
         third_party/libjpeg_turbo/jdmerge.c
         third_party/libjpeg_turbo/jdphuff.c
         third_party/libjpeg_turbo/jdpostct.c
         third_party/libjpeg_turbo/jdsample.c
         third_party/libjpeg_turbo/jdtrans.c
         third_party/libjpeg_turbo/jerror.c
         third_party/libjpeg_turbo/jerror.h
         third_party/libjpeg_turbo/jfdctflt.c
         third_party/libjpeg_turbo/jfdctfst.c
         third_party/libjpeg_turbo/jfdctint.c
         third_party/libjpeg_turbo/jidctflt.c
         third_party/libjpeg_turbo/jidctfst.c
         third_party/libjpeg_turbo/jidctint.c
         third_party/libjpeg_turbo/jidctred.c
         third_party/libjpeg_turbo/jmemmgr.c
         third_party/libjpeg_turbo/jmemnobs.c
         third_party/libjpeg_turbo/jmemsys.h
         third_party/libjpeg_turbo/jpeg_nbits_table.c
         third_party/libjpeg_turbo/jpegint.h
         third_party/libjpeg_turbo/jquant1.c
         third_party/libjpeg_turbo/jquant2.c
         third_party/libjpeg_turbo/jutils.c
         third_party/libjpeg_turbo/jversion.h)

set(CORE_SRC
    core/fpdftext/cpdf_linkextract.cpp core/fpdftext/cpdf_textpage.cpp
    core/fpdftext/cpdf_textpagefind.cpp
    core/fpdftext/unicodenormalizationdata.cpp)
list(
  APPEND CORE_SRC
         core/fpdfdoc/cba_fontmap.cpp
         core/fpdfdoc/cpdf_color_utils.cpp
         core/fpdfdoc/cpdf_iconfit.cpp
         core/fpdfdoc/cpdf_structtree.cpp
         core/fpdfdoc/cline.cpp
         core/fpdfdoc/cpdf_defaultappearance.cpp
         core/fpdfdoc/cpdf_interactiveform.cpp
         core/fpdfdoc/cpdf_variabletext.cpp
         core/fpdfdoc/cpdf_aaction.cpp
         core/fpdfdoc/cpdf_link.cpp
         core/fpdfdoc/cpdf_viewerpreferences.cpp
         core/fpdfdoc/cpdf_action.cpp
         core/fpdfdoc/cpdf_dest.cpp
         core/fpdfdoc/cpdf_linklist.cpp
         core/fpdfdoc/cpvt_fontmap.cpp
         core/fpdfdoc/cpdf_actionfields.cpp
         core/fpdfdoc/cpdf_metadata.cpp
         core/fpdfdoc/cpvt_generateap.cpp
         core/fpdfdoc/cpdf_annot.cpp
         core/fpdfdoc/cpdf_docjsactions.cpp
         core/fpdfdoc/cpvt_wordinfo.cpp
         core/fpdfdoc/cpdf_annotlist.cpp
         core/fpdfdoc/cpdf_filespec.cpp
         core/fpdfdoc/cpdf_nametree.cpp
         core/fpdfdoc/csection.cpp
         core/fpdfdoc/ctypeset.cpp
         core/fpdfdoc/cpdf_apsettings.cpp
         core/fpdfdoc/cpdf_formcontrol.cpp
         core/fpdfdoc/cpdf_numbertree.cpp
         core/fpdfdoc/cpdf_bookmark.cpp
         core/fpdfdoc/cpdf_formfield.cpp
         core/fpdfdoc/cpdf_pagelabel.cpp
         core/fpdfdoc/cpdf_bookmarktree.cpp
         core/fpdfdoc/cpdf_structelement.cpp)

# core/fxcrt
list(
  APPEND CORE_SRC
         core/fxcrt/bytestring.cpp
         core/fxcrt/cfx_utf8decoder.cpp
         core/fxcrt/fx_number.cpp
         core/fxcrt/cfx_utf8encoder.cpp
         core/fxcrt/cfx_binarybuf.cpp
         core/fxcrt/cfx_widetextbuf.cpp
         core/fxcrt/fx_random.cpp
         core/fxcrt/cfx_bitstream.cpp
         core/fxcrt/fx_bidi.cpp
         core/fxcrt/fx_stream.cpp
         core/fxcrt/cfx_datetime.cpp
         core/fxcrt/fx_string.cpp
         core/fxcrt/cfx_fileaccess_posix.cpp
         core/fxcrt/fx_codepage.cpp
         core/fxcrt/fx_coordinates.cpp
         core/fxcrt/fx_system.cpp
         core/fxcrt/cfx_memorystream.cpp
         core/fxcrt/widestring.cpp
         core/fxcrt/fx_extension.cpp
         core/fxcrt/fx_unicode.cpp
         core/fxcrt/cfx_readonlymemorystream.cpp
         core/fxcrt/cfx_seekablestreamproxy.cpp
         core/fxcrt/fx_memory.cpp
         core/fxcrt/observed_ptr.cpp)

if(WIN32)
  # list(APPEND CORE_SRC core/fxcrt/cfx_fileaccess_windows.cpp)
else()
  # list(APPEND CORE_SRC core/fxcrt/cfx_fileaccess_posix.cpp)
endif()

# core/fxge
list(
  APPEND CORE_SRC
         core/fxge/cfx_cliprgn.cpp
         core/fxge/cfx_fontmapper.cpp
         core/fxge/cfx_graphstatedata.cpp
         core/fxge/fx_font.cpp
         core/fxge/scoped_font_transform.cpp
         core/fxge/cfx_color.cpp
         core/fxge/cfx_fontmgr.cpp
         core/fxge/cfx_pathdata.cpp
         core/fxge/text_char_pos.cpp
         core/fxge/cfx_face.cpp
         core/fxge/cfx_gemodule.cpp
         core/fxge/cfx_renderdevice.cpp
         core/fxge/fx_ge_fontmap.cpp
         core/fxge/text_glyph_pos.cpp
         core/fxge/cfx_folderfontinfo.cpp
         core/fxge/cfx_glyphbitmap.cpp
         core/fxge/cfx_substfont.cpp
         core/fxge/fx_ge_linux.cpp
         core/fxge/cfx_fontcache.cpp
         core/fxge/cfx_glyphcache.cpp
         core/fxge/cfx_unicodeencoding.cpp
         core/fxge/cfx_font.cpp
         core/fxge/cfx_graphstate.cpp
         core/fxge/renderdevicedriver_iface.cpp)
# core/fxge/cfx_unicodeencodingex.cpp

# third_party/skia_shared. used by
# core/fpdfapi/edit/cpdf_contentstream_write_utils.cpp
list(APPEND CORE_SRC third_party/skia_shared/SkFloatToDecimal.cpp
            third_party/skia_shared/SkFloatToDecimal.h)
# core/fpdfapi/edit
list(
  APPEND CORE_SRC
         core/fpdfapi/edit/cpdf_contentstream_write_utils.cpp
         core/fpdfapi/edit/cpdf_contentstream_write_utils.h
         core/fpdfapi/edit/cpdf_creator.cpp
         core/fpdfapi/edit/cpdf_creator.h
         core/fpdfapi/edit/cpdf_pagecontentgenerator.cpp
         core/fpdfapi/edit/cpdf_pagecontentgenerator.h
         core/fpdfapi/edit/cpdf_pagecontentmanager.cpp
         core/fpdfapi/edit/cpdf_pagecontentmanager.h
         core/fpdfapi/edit/cpdf_stringarchivestream.cpp
         core/fpdfapi/edit/cpdf_stringarchivestream.h)
# core/fpdfapi/parse
list(
  APPEND CORE_SRC
         core/fpdfapi/parser/cfdf_document.cpp
         core/fpdfapi/parser/cpdf_object_walker.cpp
         core/fpdfapi/parser/cpdf_array.cpp
         core/fpdfapi/parser/cpdf_page_object_avail.cpp
         core/fpdfapi/parser/cpdf_boolean.cpp
         core/fpdfapi/parser/cpdf_cross_ref_avail.cpp
         core/fpdfapi/parser/cpdf_parser.cpp
         core/fpdfapi/parser/cpdf_cross_ref_table.cpp
         core/fpdfapi/parser/cpdf_crypto_handler.cpp
         core/fpdfapi/parser/cpdf_read_validator.cpp
         core/fpdfapi/parser/cpdf_data_avail.cpp
         core/fpdfapi/parser/cpdf_dictionary.cpp
         core/fpdfapi/parser/cpdf_reference.cpp
         core/fpdfapi/parser/cpdf_document.cpp
         core/fpdfapi/parser/cpdf_security_handler.cpp
         core/fpdfapi/parser/cpdf_encryptor.cpp
         core/fpdfapi/parser/cpdf_seekablemultistream.cpp
         core/fpdfapi/parser/cpdf_flateencoder.cpp
         core/fpdfapi/parser/cpdf_hint_tables.cpp
         core/fpdfapi/parser/cpdf_simple_parser.cpp
         core/fpdfapi/parser/cpdf_indirect_object_holder.cpp
         core/fpdfapi/parser/cpdf_stream_acc.cpp
         core/fpdfapi/parser/cpdf_linearized_header.cpp
         core/fpdfapi/parser/cpdf_stream.cpp
         core/fpdfapi/parser/cpdf_name.cpp
         core/fpdfapi/parser/cpdf_string.cpp
         core/fpdfapi/parser/cpdf_null.cpp
         core/fpdfapi/parser/cpdf_syntax_parser.cpp
         core/fpdfapi/parser/cpdf_number.cpp
         core/fpdfapi/parser/cpdf_object_avail.cpp
         core/fpdfapi/parser/fpdf_parser_decode.cpp
         core/fpdfapi/parser/cpdf_object.cpp
         core/fpdfapi/parser/cpdf_object_stream.cpp
         core/fpdfapi/parser/fpdf_parser_utility.cpp)

# core/fpdfapi/render
list(
  APPEND CORE_SRC
         core/fpdfapi/render/cpdf_charposlist.cpp
         core/fpdfapi/render/cpdf_pagerendercache.cpp
         core/fpdfapi/render/cpdf_textrenderer.cpp
         core/fpdfapi/render/cpdf_devicebuffer.cpp
         core/fpdfapi/render/cpdf_pagerendercontext.cpp
         core/fpdfapi/render/cpdf_type3cache.cpp
         core/fpdfapi/render/cpdf_docrenderdata.cpp
         core/fpdfapi/render/cpdf_progressiverenderer.cpp
         core/fpdfapi/render/cpdf_type3glyphmap.cpp
         core/fpdfapi/render/cpdf_rendercontext.cpp
         core/fpdfapi/render/cpdf_imagecacheentry.cpp
         core/fpdfapi/render/cpdf_renderoptions.cpp
         core/fpdfapi/render/cpdf_imageloader.cpp
         core/fpdfapi/render/cpdf_renderstatus.cpp
         core/fpdfapi/render/cpdf_imagerenderer.cpp
         core/fpdfapi/render/cpdf_scaledrenderbuffer.cpp)
# win32 core/fpdfapi/render/cpdf_windowsrenderdevice.cpp fpdfsdk

# core/fpdfapi/font
list(
  APPEND CORE_SRC
         core/fpdfapi/font/cfx_cttgsubtable.cpp
         core/fpdfapi/font/cpdf_cmap.cpp
         core/fpdfapi/font/cpdf_fontencoding.cpp
         core/fpdfapi/font/cpdf_truetypefont.cpp
         core/fpdfapi/font/cfx_stockfontarray.cpp
         core/fpdfapi/font/cpdf_cmapmanager.cpp
         core/fpdfapi/font/cpdf_fontglobals.cpp
         core/fpdfapi/font/cpdf_type1font.cpp
         core/fpdfapi/font/cpdf_cid2unicodemap.cpp
         core/fpdfapi/font/cpdf_cmapparser.cpp
         core/fpdfapi/font/cpdf_simplefont.cpp
         core/fpdfapi/font/cpdf_type3char.cpp
         core/fpdfapi/font/cpdf_cidfont.cpp
         core/fpdfapi/font/cpdf_tounicodemap.cpp
         core/fpdfapi/font/cpdf_type3font.cpp
         core/fpdfapi/font/cpdf_font.cpp)

# core/fpdfapi/page
list(
  APPEND CORE_SRC
         core/fpdfapi/page/cpdf_allstates.cpp
         core/fpdfapi/page/cpdf_generalstate.cpp
         core/fpdfapi/page/cpdf_clippath.cpp
         core/fpdfapi/page/cpdf_graphicstates.cpp
         core/fpdfapi/page/cpdf_psfunc.cpp
         core/fpdfapi/page/cpdf_color.cpp
         core/fpdfapi/page/cpdf_iccprofile.cpp
         core/fpdfapi/page/cpdf_sampledfunc.cpp
         core/fpdfapi/page/cpdf_colorspace.cpp
         core/fpdfapi/page/cpdf_image.cpp
         core/fpdfapi/page/cpdf_shadingobject.cpp
         core/fpdfapi/page/cpdf_colorstate.cpp
         core/fpdfapi/page/cpdf_imageobject.cpp
         core/fpdfapi/page/cpdf_shadingpattern.cpp
         core/fpdfapi/page/cpdf_contentmarkitem.cpp
         core/fpdfapi/page/cpdf_meshstream.cpp
         core/fpdfapi/page/cpdf_stitchfunc.cpp
         core/fpdfapi/page/cpdf_contentmarks.cpp
         core/fpdfapi/page/cpdf_occontext.cpp
         core/fpdfapi/page/cpdf_streamcontentparser.cpp
         core/fpdfapi/page/cpdf_contentparser.cpp
         core/fpdfapi/page/cpdf_page.cpp
         core/fpdfapi/page/cpdf_devicecs.cpp
         core/fpdfapi/page/cpdf_pagemodule.cpp
         core/fpdfapi/page/cpdf_streamparser.cpp
         core/fpdfapi/page/cpdf_pageobject.cpp
         core/fpdfapi/page/cpdf_dibbase.cpp
         core/fpdfapi/page/cpdf_pageobjectholder.cpp
         core/fpdfapi/page/cpdf_textobject.cpp
         core/fpdfapi/page/cpdf_dibtransferfunc.cpp
         core/fpdfapi/page/cpdf_textstate.cpp
         core/fpdfapi/page/cpdf_docpagedata.cpp
         core/fpdfapi/page/cpdf_path.cpp
         core/fpdfapi/page/cpdf_tilingpattern.cpp
         core/fpdfapi/page/cpdf_expintfunc.cpp
         core/fpdfapi/page/cpdf_pathobject.cpp
         core/fpdfapi/page/cpdf_transferfunc.cpp
         core/fpdfapi/page/cpdf_form.cpp
         core/fpdfapi/page/cpdf_pattern.cpp
         core/fpdfapi/page/cpdf_transparency.cpp
         core/fpdfapi/page/cpdf_formobject.cpp
         core/fpdfapi/page/cpdf_patterncs.cpp
         core/fpdfapi/page/cpdf_function.cpp
         core/fpdfapi/page/cpdf_psengine.cpp)

list(
  APPEND CORE_SRC
         fpdfsdk/cfx_systemhandler.cpp
         fpdfsdk/fpdf_attachment.cpp
         fpdfsdk/fpdf_javascript.cpp
         fpdfsdk/cpdf_annotcontext.cpp
         fpdfsdk/cpdfsdk_actionhandler.cpp
         fpdfsdk/fpdf_catalog.cpp
         fpdfsdk/fpdf_ppo.cpp
         fpdfsdk/cpdfsdk_annot.cpp
         fpdfsdk/cpdfsdk_annothandlermgr.cpp
         fpdfsdk/fpdf_dataavail.cpp
         fpdfsdk/fpdf_progressive.cpp
         fpdfsdk/cpdfsdk_annotiteration.cpp
         fpdfsdk/fpdf_save.cpp
         fpdfsdk/cpdfsdk_annotiterator.cpp
         fpdfsdk/fpdf_doc.cpp
         fpdfsdk/fpdf_searchex.cpp
         fpdfsdk/cpdfsdk_appstream.cpp
         fpdfsdk/cpdfsdk_baannot.cpp
         fpdfsdk/fpdf_structtree.cpp
         fpdfsdk/cpdfsdk_baannothandler.cpp
         fpdfsdk/fpdf_editimg.cpp
         fpdfsdk/cpdfsdk_customaccess.cpp
         fpdfsdk/fpdf_sysfontinfo.cpp
         fpdfsdk/cpdfsdk_fieldaction.cpp
         fpdfsdk/fpdf_editpage.cpp
         fpdfsdk/cpdfsdk_filewriteadapter.cpp
         fpdfsdk/fpdf_text.cpp
         fpdfsdk/cpdfsdk_formfillenvironment.cpp
         fpdfsdk/fpdf_editpath.cpp
         fpdfsdk/cpdfsdk_helpers.cpp
         fpdfsdk/fpdf_thumbnail.cpp
         fpdfsdk/cpdfsdk_interactiveform.cpp
         fpdfsdk/fpdf_edittext.cpp
         fpdfsdk/cpdfsdk_pageview.cpp
         fpdfsdk/fpdf_transformpage.cpp
         fpdfsdk/cpdfsdk_widget.cpp
         fpdfsdk/fpdf_ext.cpp
         fpdfsdk/cpdfsdk_widgethandler.cpp
         fpdfsdk/fpdf_view.cpp
         fpdfsdk/fpdf_flatten.cpp
         fpdfsdk/ipdfsdk_pauseadapter.cpp
         fpdfsdk/fpdf_annot.cpp
         fpdfsdk/fpdf_formfill.cpp)

# fpdfsdk/formfiller
list(
  APPEND CORE_SRC
         fpdfsdk/formfiller/cffl_button.cpp
         fpdfsdk/formfiller/cffl_formfiller.cpp
         fpdfsdk/formfiller/cffl_pushbutton.cpp
         fpdfsdk/formfiller/cffl_textobject.cpp
         fpdfsdk/formfiller/cffl_checkbox.cpp
         fpdfsdk/formfiller/cffl_interactiveformfiller.cpp
         fpdfsdk/formfiller/cffl_radiobutton.cpp
         fpdfsdk/formfiller/cffl_combobox.cpp
         fpdfsdk/formfiller/cffl_listbox.cpp
         fpdfsdk/formfiller/cffl_textfield.cpp)

# fpdfsdk/pwl
list(
  APPEND CORE_SRC
         fpdfsdk/pwl/cpwl_button.cpp
         fpdfsdk/pwl/cpwl_edit.cpp
         fpdfsdk/pwl/cpwl_icon.cpp
         fpdfsdk/pwl/cpwl_special_button.cpp
         fpdfsdk/pwl/cpwl_caret.cpp
         fpdfsdk/pwl/cpwl_edit_ctrl.cpp
         fpdfsdk/pwl/cpwl_list_box.cpp
         fpdfsdk/pwl/cpwl_timer.cpp
         fpdfsdk/pwl/cpwl_combo_box.cpp
         fpdfsdk/pwl/cpwl_list_impl.cpp
         fpdfsdk/pwl/cpwl_timer_handler.cpp
         fpdfsdk/pwl/cpwl_edit_impl.cpp
         fpdfsdk/pwl/cpwl_scroll_bar.cpp
         fpdfsdk/pwl/cpwl_wnd.cpp)

# fxjs wrap of V8
list(APPEND CORE_SRC fxjs/cjs_runtimestub.cpp fxjs/ijs_runtime.cpp
            fxjs/cjs_event_context_stub.cpp)
# fxjs/cfx_globalarray.cpp            fxjs/cjs_app.cpp
# fxjs/cjs_eventrecorder.cpp  fxjs/cjs_publicmethods.cpp fxjs/cjs_util.cpp
# fxjs/cfx_globaldata.cpp             fxjs/cjs_border.cpp fxjs/cjs_field.cpp
# fxjs/cjs_color.cpp               fxjs/cjs_font.cpp fxjs/cjs_zoomtype.cpp
# fxjs/cfxjs_engine.cpp               fxjs/cjs_console.cpp
# fxjs/cjs_globalarrays.cpp   fxjs/cjs_result.cpp fxjs/fx_date_helpers.cpp
# fxjs/cjs_delaydata.cpp fxjs/cjs_globalconsts.cpp   fxjs/cjs_runtime.cpp
# fxjs/global_timer.cpp fxjs/cjs_display.cpp             fxjs/cjs_global.cpp
# fxjs/cfx_keyvalue.cpp               fxjs/cjs_document.cpp
# fxjs/cjs_highlight.cpp      fxjs/cjs_scalehow.cpp fxjs/js_define.cpp
# fxjs/cfx_v8.cpp fxjs/cjs_icon.cpp           fxjs/cjs_scalewhen.cpp
# fxjs/js_resources.cpp fxjs/cjs_object.cpp         fxjs/cjs_style.cpp
# fxjs/cjs_annot.cpp                  fxjs/cjs_event.cpp fxjs/cjs_position.cpp
# fxjs/cjs_timerobj.cpp )

# core/fxge/dib
list(
  APPEND CORE_SRC
         core/fxge/dib/cfx_bitmapcomposer.cpp
         core/fxge/dib/cfx_dibbase.cpp
         core/fxge/dib/cfx_filtereddib.cpp
         core/fxge/dib/cfx_scanlinecompositor.cpp
         core/fxge/dib/cfx_bitmapstorer.cpp
         core/fxge/dib/cfx_dibextractor.cpp
         core/fxge/dib/cfx_imagerenderer.cpp
         core/fxge/dib/cstretchengine.cpp
         core/fxge/dib/cfx_cmyk_to_srgb.cpp
         core/fxge/dib/cfx_dibitmap.cpp
         core/fxge/dib/cfx_imagestretcher.cpp
         core/fxge/dib/cfx_imagetransformer.cpp
         core/fxge/dib/fx_dib_main.cpp)
# core/fxge/agg
list(APPEND CORE_SRC core/fxge/agg/fx_agg_driver.cpp)

# core/fxge/fontdata/chromefontdata
list(
  APPEND CORE_SRC
         core/fxge/fontdata/chromefontdata/FoxitDingbats.cpp
         core/fxge/fontdata/chromefontdata/FoxitSansItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitFixedBold.cpp
         core/fxge/fontdata/chromefontdata/FoxitSansMM.cpp
         core/fxge/fontdata/chromefontdata/FoxitFixedBoldItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitSerifBold.cpp
         core/fxge/fontdata/chromefontdata/FoxitFixed.cpp
         core/fxge/fontdata/chromefontdata/FoxitSerifBoldItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitFixedItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitSerif.cpp
         core/fxge/fontdata/chromefontdata/FoxitSansBold.cpp
         core/fxge/fontdata/chromefontdata/FoxitSerifItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitSansBoldItalic.cpp
         core/fxge/fontdata/chromefontdata/FoxitSerifMM.cpp
         core/fxge/fontdata/chromefontdata/FoxitSans.cpp
         core/fxge/fontdata/chromefontdata/FoxitSymbol.cpp)

# core/fxge/freetype
list(APPEND CORE_SRC core/fxge/freetype/fx_freetype.cpp)

list(APPEND CORE_SRC core/fdrm/fx_crypt_aes.cpp core/fdrm/fx_crypt.cpp
            core/fdrm/fx_crypt_sha.cpp)

# core/fxcodec
list(
  APPEND CORE_SRC
         core/fxcodec/basic/basicmodule.cpp
         core/fxcodec/basic/basicmodule.h
         core/fxcodec/cfx_codec_memory.cpp
         core/fxcodec/cfx_codec_memory.h
         core/fxcodec/codec_module_iface.h
         core/fxcodec/fax/faxmodule.cpp
         core/fxcodec/fax/faxmodule.h
         core/fxcodec/flate/flatemodule.cpp
         core/fxcodec/flate/flatemodule.h
         core/fxcodec/fx_codec.cpp
         core/fxcodec/fx_codec.h
         core/fxcodec/fx_codec_def.h
         core/fxcodec/icc/iccmodule.cpp
         core/fxcodec/icc/iccmodule.h
         core/fxcodec/jbig2/JBig2_ArithDecoder.cpp
         core/fxcodec/jbig2/JBig2_ArithDecoder.h
         core/fxcodec/jbig2/JBig2_ArithIntDecoder.cpp
         core/fxcodec/jbig2/JBig2_ArithIntDecoder.h
         core/fxcodec/jbig2/JBig2_BitStream.cpp
         core/fxcodec/jbig2/JBig2_BitStream.h
         core/fxcodec/jbig2/JBig2_Context.cpp
         core/fxcodec/jbig2/JBig2_Context.h
         core/fxcodec/jbig2/JBig2_Define.h
         core/fxcodec/jbig2/JBig2_DocumentContext.cpp
         core/fxcodec/jbig2/JBig2_DocumentContext.h
         core/fxcodec/jbig2/JBig2_GrdProc.cpp
         core/fxcodec/jbig2/JBig2_GrdProc.h
         core/fxcodec/jbig2/JBig2_GrrdProc.cpp
         core/fxcodec/jbig2/JBig2_GrrdProc.h
         core/fxcodec/jbig2/JBig2_HtrdProc.cpp
         core/fxcodec/jbig2/JBig2_HtrdProc.h
         core/fxcodec/jbig2/JBig2_HuffmanDecoder.cpp
         core/fxcodec/jbig2/JBig2_HuffmanDecoder.h
         core/fxcodec/jbig2/JBig2_HuffmanTable.cpp
         core/fxcodec/jbig2/JBig2_HuffmanTable.h
         core/fxcodec/jbig2/JBig2_Image.cpp
         core/fxcodec/jbig2/JBig2_Image.h
         core/fxcodec/jbig2/JBig2_Page.h
         core/fxcodec/jbig2/JBig2_PatternDict.cpp
         core/fxcodec/jbig2/JBig2_PatternDict.h
         core/fxcodec/jbig2/JBig2_PddProc.cpp
         core/fxcodec/jbig2/JBig2_PddProc.h
         core/fxcodec/jbig2/JBig2_SddProc.cpp
         core/fxcodec/jbig2/JBig2_SddProc.h
         core/fxcodec/jbig2/JBig2_Segment.cpp
         core/fxcodec/jbig2/JBig2_Segment.h
         core/fxcodec/jbig2/JBig2_SymbolDict.cpp
         core/fxcodec/jbig2/JBig2_SymbolDict.h
         core/fxcodec/jbig2/JBig2_TrdProc.cpp
         core/fxcodec/jbig2/JBig2_TrdProc.h
         core/fxcodec/jbig2/jbig2module.cpp
         core/fxcodec/jbig2/jbig2module.h
         core/fxcodec/jpeg/jpegmodule.cpp
         core/fxcodec/jpeg/jpegmodule.h
         core/fxcodec/jpx/cjpx_decoder.cpp
         core/fxcodec/jpx/cjpx_decoder.h
         core/fxcodec/jpx/jpx_decode_utils.cpp
         core/fxcodec/jpx/jpx_decode_utils.h
         core/fxcodec/jpx/jpxmodule.cpp
         core/fxcodec/jpx/jpxmodule.h
         core/fxcodec/scanlinedecoder.cpp
         core/fxcodec/scanlinedecoder.h)

list(
  APPEND CORE_SRC
         core/fxcrt/xml/cfx_xmlchardata.cpp
         core/fxcrt/xml/cfx_xmlelement.cpp
         core/fxcrt/xml/cfx_xmlnode.cpp
         core/fxcrt/xml/cfx_xmltext.cpp
         core/fxcrt/xml/cfx_xmldocument.cpp
         core/fxcrt/xml/cfx_xmlinstruction.cpp
         core/fxcrt/xml/cfx_xmlparser.cpp)

# third_party/base
list(
  APPEND
    CORE_SRC
    third_party/base/allocator/partition_allocator/address_space_randomization.cc
    third_party/base/allocator/partition_allocator/address_space_randomization.h
    third_party/base/allocator/partition_allocator/oom.h
    third_party/base/allocator/partition_allocator/oom_callback.cc
    third_party/base/allocator/partition_allocator/oom_callback.h
    third_party/base/allocator/partition_allocator/page_allocator.cc
    third_party/base/allocator/partition_allocator/page_allocator.h
    third_party/base/allocator/partition_allocator/page_allocator_constants.h
    third_party/base/allocator/partition_allocator/page_allocator_internal.h
    third_party/base/allocator/partition_allocator/page_allocator_internals_posix.h
    third_party/base/allocator/partition_allocator/page_allocator_internals_win.h
    third_party/base/allocator/partition_allocator/partition_alloc.cc
    third_party/base/allocator/partition_allocator/partition_alloc.h
    third_party/base/allocator/partition_allocator/partition_alloc_constants.h
    third_party/base/allocator/partition_allocator/partition_bucket.cc
    third_party/base/allocator/partition_allocator/partition_bucket.h
    third_party/base/allocator/partition_allocator/partition_cookie.h
    third_party/base/allocator/partition_allocator/partition_direct_map_extent.h
    third_party/base/allocator/partition_allocator/partition_freelist_entry.h
    third_party/base/allocator/partition_allocator/partition_oom.cc
    third_party/base/allocator/partition_allocator/partition_oom.h
    third_party/base/allocator/partition_allocator/partition_page.cc
    third_party/base/allocator/partition_allocator/partition_page.h
    third_party/base/allocator/partition_allocator/partition_root_base.cc
    third_party/base/allocator/partition_allocator/partition_root_base.h
    third_party/base/allocator/partition_allocator/spin_lock.cc
    third_party/base/allocator/partition_allocator/spin_lock.h
    third_party/base/base_export.h
    third_party/base/bits.h
    third_party/base/compiler_specific.h
    third_party/base/debug/alias.cc
    third_party/base/debug/alias.h
    third_party/base/logging.h
    third_party/base/numerics/safe_conversions.h
    third_party/base/numerics/safe_conversions_arm_impl.h
    third_party/base/numerics/safe_conversions_impl.h
    third_party/base/numerics/safe_math.h
    third_party/base/numerics/safe_math_impl.h
    third_party/base/optional.h
    third_party/base/span.h
    third_party/base/stl_util.h
    third_party/base/sys_byteorder.h
    third_party/base/template_util.h)

list(
  APPEND CORE_SRC
         third_party/lcms/include/lcms2.h
         third_party/lcms/include/lcms2_plugin.h
         third_party/lcms/src/cmsalpha.c
         third_party/lcms/src/cmscam02.c
         third_party/lcms/src/cmscgats.c
         third_party/lcms/src/cmscnvrt.c
         third_party/lcms/src/cmserr.c
         third_party/lcms/src/cmsgamma.c
         third_party/lcms/src/cmsgmt.c
         third_party/lcms/src/cmshalf.c
         third_party/lcms/src/cmsintrp.c
         third_party/lcms/src/cmsio0.c
         third_party/lcms/src/cmsio1.c
         third_party/lcms/src/cmslut.c
         third_party/lcms/src/cmsmd5.c
         third_party/lcms/src/cmsmtrx.c
         third_party/lcms/src/cmsnamed.c
         third_party/lcms/src/cmsopt.c
         third_party/lcms/src/cmspack.c
         third_party/lcms/src/cmspcs.c
         third_party/lcms/src/cmsplugin.c
         third_party/lcms/src/cmsps2.c
         third_party/lcms/src/cmssamp.c
         third_party/lcms/src/cmssm.c
         third_party/lcms/src/cmstypes.c
         third_party/lcms/src/cmsvirt.c
         third_party/lcms/src/cmswtpnt.c
         third_party/lcms/src/cmsxform.c)

list(
  APPEND CORE_SRC
         core/fpdfapi/cmaps/CNS1/Adobe-CNS1-UCS2_5.cpp
         core/fpdfapi/cmaps/CNS1/B5pc-H_0.cpp
         core/fpdfapi/cmaps/CNS1/B5pc-V_0.cpp
         core/fpdfapi/cmaps/CNS1/CNS-EUC-H_0.cpp
         core/fpdfapi/cmaps/CNS1/CNS-EUC-V_0.cpp
         core/fpdfapi/cmaps/CNS1/ETen-B5-H_0.cpp
         core/fpdfapi/cmaps/CNS1/ETen-B5-V_0.cpp
         core/fpdfapi/cmaps/CNS1/ETenms-B5-H_0.cpp
         core/fpdfapi/cmaps/CNS1/ETenms-B5-V_0.cpp
         core/fpdfapi/cmaps/CNS1/HKscs-B5-H_5.cpp
         core/fpdfapi/cmaps/CNS1/HKscs-B5-V_5.cpp
         core/fpdfapi/cmaps/CNS1/UniCNS-UCS2-H_3.cpp
         core/fpdfapi/cmaps/CNS1/UniCNS-UCS2-V_3.cpp
         core/fpdfapi/cmaps/CNS1/UniCNS-UTF16-H_0.cpp
         core/fpdfapi/cmaps/CNS1/cmaps_cns1.cpp
         core/fpdfapi/cmaps/GB1/Adobe-GB1-UCS2_5.cpp
         core/fpdfapi/cmaps/GB1/GB-EUC-H_0.cpp
         core/fpdfapi/cmaps/GB1/GB-EUC-V_0.cpp
         core/fpdfapi/cmaps/GB1/GBK-EUC-H_2.cpp
         core/fpdfapi/cmaps/GB1/GBK-EUC-V_2.cpp
         core/fpdfapi/cmaps/GB1/GBK2K-H_5.cpp
         core/fpdfapi/cmaps/GB1/GBK2K-V_5.cpp
         core/fpdfapi/cmaps/GB1/GBKp-EUC-H_2.cpp
         core/fpdfapi/cmaps/GB1/GBKp-EUC-V_2.cpp
         core/fpdfapi/cmaps/GB1/GBpc-EUC-H_0.cpp
         core/fpdfapi/cmaps/GB1/GBpc-EUC-V_0.cpp
         core/fpdfapi/cmaps/GB1/UniGB-UCS2-H_4.cpp
         core/fpdfapi/cmaps/GB1/UniGB-UCS2-V_4.cpp
         core/fpdfapi/cmaps/GB1/cmaps_gb1.cpp
         core/fpdfapi/cmaps/Japan1/83pv-RKSJ-H_1.cpp
         core/fpdfapi/cmaps/Japan1/90ms-RKSJ-H_2.cpp
         core/fpdfapi/cmaps/Japan1/90ms-RKSJ-V_2.cpp
         core/fpdfapi/cmaps/Japan1/90msp-RKSJ-H_2.cpp
         core/fpdfapi/cmaps/Japan1/90msp-RKSJ-V_2.cpp
         core/fpdfapi/cmaps/Japan1/90pv-RKSJ-H_1.cpp
         core/fpdfapi/cmaps/Japan1/Add-RKSJ-H_1.cpp
         core/fpdfapi/cmaps/Japan1/Add-RKSJ-V_1.cpp
         core/fpdfapi/cmaps/Japan1/Adobe-Japan1-UCS2_4.cpp
         core/fpdfapi/cmaps/Japan1/EUC-H_1.cpp
         core/fpdfapi/cmaps/Japan1/EUC-V_1.cpp
         core/fpdfapi/cmaps/Japan1/Ext-RKSJ-H_2.cpp
         core/fpdfapi/cmaps/Japan1/Ext-RKSJ-V_2.cpp
         core/fpdfapi/cmaps/Japan1/H_1.cpp
         core/fpdfapi/cmaps/Japan1/UniJIS-UCS2-HW-H_4.cpp
         core/fpdfapi/cmaps/Japan1/UniJIS-UCS2-HW-V_4.cpp
         core/fpdfapi/cmaps/Japan1/UniJIS-UCS2-H_4.cpp
         core/fpdfapi/cmaps/Japan1/UniJIS-UCS2-V_4.cpp
         core/fpdfapi/cmaps/Japan1/V_1.cpp
         core/fpdfapi/cmaps/Japan1/cmaps_japan1.cpp
         core/fpdfapi/cmaps/Korea1/Adobe-Korea1-UCS2_2.cpp
         core/fpdfapi/cmaps/Korea1/KSC-EUC-H_0.cpp
         core/fpdfapi/cmaps/Korea1/KSC-EUC-V_0.cpp
         core/fpdfapi/cmaps/Korea1/KSCms-UHC-HW-H_1.cpp
         core/fpdfapi/cmaps/Korea1/KSCms-UHC-HW-V_1.cpp
         core/fpdfapi/cmaps/Korea1/KSCms-UHC-H_1.cpp
         core/fpdfapi/cmaps/Korea1/KSCms-UHC-V_1.cpp
         core/fpdfapi/cmaps/Korea1/KSCpc-EUC-H_0.cpp
         core/fpdfapi/cmaps/Korea1/UniKS-UCS2-H_1.cpp
         core/fpdfapi/cmaps/Korea1/UniKS-UCS2-V_1.cpp
         core/fpdfapi/cmaps/Korea1/UniKS-UTF16-H_0.cpp
         core/fpdfapi/cmaps/Korea1/cmaps_korea1.cpp
         core/fpdfapi/cmaps/cmap_int.h
         core/fpdfapi/cmaps/fpdf_cmaps.cpp)

include_directories(third_party/icu/source/common/)
# include_directories(third_party/icu/source/i18n)
add_definitions(
  -DHAVE_DLOPEN=0
  -DUCONFIG_ONLY_HTML_CONVERSION=1
  -DUCONFIG_USE_WINDOWS_LCID_MAPPING_API=0
  -DU_CHARSET_IS_UTF8=1
  -DU_USING_ICU_NAMESPACE=0
  -DU_ENABLE_DYLOAD=0
  -DU_STATIC_IMPLEMENTATION
  -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_STATIC
  -DUCHAR_TYPE=uint16_t) # -DU_I18N_IMPLEMENTATION)

# generate icu data
# common/icudtl.dat    icudtl_dat.S
# third_party/icu/scripts/make_data_assembly.py source/stubdata/stubdata.cpp
add_definitions(-DU_ICUDATAENTRY_IN_COMMON) # -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA
                                            # _FILE
execute_process(
  COMMAND
    python ${CMAKE_SOURCE_DIR}/third_party/icu/scripts/make_data_assembly.py
    ${CMAKE_SOURCE_DIR}/third_party/icu/common/icudtl.dat
    ${CMAKE_SOURCE_DIR}/third_party/icu/common/icudtl_data.S)
# enable_language(ASM_NASM)
enable_language(ASM)
add_definitions(-DU_HIDE_DATA_SYMBOL)
list(
  APPEND CORE_SRC
         third_party/icu/common/icudtl_data.S
         third_party/icu/source/common/appendable.cpp
         third_party/icu/source/common/bmpset.cpp
         third_party/icu/source/common/bmpset.h
         third_party/icu/source/common/brkeng.cpp
         third_party/icu/source/common/brkeng.h
         third_party/icu/source/common/brkiter.cpp
         third_party/icu/source/common/bytesinkutil.cpp
         third_party/icu/source/common/bytesinkutil.h
         third_party/icu/source/common/bytestream.cpp
         third_party/icu/source/common/bytestriebuilder.cpp
         third_party/icu/source/common/bytestrie.cpp
         third_party/icu/source/common/bytestrieiterator.cpp
         third_party/icu/source/common/caniter.cpp
         third_party/icu/source/common/capi_helper.h
         third_party/icu/source/common/characterproperties.cpp
         third_party/icu/source/common/chariter.cpp
         third_party/icu/source/common/charstr.cpp
         third_party/icu/source/common/charstr.h
         third_party/icu/source/common/cmemory.cpp
         third_party/icu/source/common/cmemory.h
         third_party/icu/source/common/cpputils.h
         third_party/icu/source/common/cstr.cpp
         third_party/icu/source/common/cstr.h
         third_party/icu/source/common/cstring.cpp
         third_party/icu/source/common/cstring.h
         third_party/icu/source/common/cwchar.cpp
         third_party/icu/source/common/cwchar.h
         third_party/icu/source/common/dictbe.cpp
         third_party/icu/source/common/dictbe.h
         third_party/icu/source/common/dictionarydata.cpp
         third_party/icu/source/common/dictionarydata.h
         third_party/icu/source/common/dtintrv.cpp
         third_party/icu/source/common/edits.cpp
         third_party/icu/source/common/errorcode.cpp
         third_party/icu/source/common/filteredbrk.cpp
         third_party/icu/source/common/filterednormalizer2.cpp
         third_party/icu/source/common/hash.h
         third_party/icu/source/common/icudataver.cpp
         third_party/icu/source/common/icuplug.cpp
         third_party/icu/source/common/icuplugimp.h
         third_party/icu/source/common/loadednormalizer2impl.cpp
         third_party/icu/source/common/localebuilder.cpp
         third_party/icu/source/common/localsvc.h
         third_party/icu/source/common/locavailable.cpp
         third_party/icu/source/common/locbased.cpp
         third_party/icu/source/common/locbased.h
         third_party/icu/source/common/locdispnames.cpp
         third_party/icu/source/common/locdspnm.cpp
         third_party/icu/source/common/locid.cpp
         third_party/icu/source/common/loclikely.cpp
         third_party/icu/source/common/locmap.cpp
         third_party/icu/source/common/locmap.h
         third_party/icu/source/common/locresdata.cpp
         third_party/icu/source/common/locutil.cpp
         third_party/icu/source/common/locutil.h
         third_party/icu/source/common/messageimpl.h
         third_party/icu/source/common/messagepattern.cpp
         third_party/icu/source/common/msvcres.h
         third_party/icu/source/common/mutex.h
         third_party/icu/source/common/norm2allmodes.h
         third_party/icu/source/common/norm2_nfc_data.h
         third_party/icu/source/common/normalizer2.cpp
         third_party/icu/source/common/normalizer2impl.cpp
         third_party/icu/source/common/normalizer2impl.h
         third_party/icu/source/common/normlzr.cpp
         third_party/icu/source/common/parsepos.cpp
         third_party/icu/source/common/patternprops.cpp
         third_party/icu/source/common/patternprops.h
         third_party/icu/source/common/pluralmap.cpp
         third_party/icu/source/common/pluralmap.h
         third_party/icu/source/common/propname.cpp
         third_party/icu/source/common/propname_data.h
         third_party/icu/source/common/propname.h
         third_party/icu/source/common/propsvec.cpp
         third_party/icu/source/common/propsvec.h
         third_party/icu/source/common/punycode.cpp
         third_party/icu/source/common/punycode.h
         third_party/icu/source/common/putil.cpp
         third_party/icu/source/common/putilimp.h
         third_party/icu/source/common/rbbi_cache.cpp
         third_party/icu/source/common/rbbi_cache.h
         third_party/icu/source/common/rbbi.cpp
         third_party/icu/source/common/rbbidata.cpp
         third_party/icu/source/common/rbbidata.h
         third_party/icu/source/common/rbbinode.cpp
         third_party/icu/source/common/rbbinode.h
         third_party/icu/source/common/rbbirb.cpp
         third_party/icu/source/common/rbbirb.h
         third_party/icu/source/common/rbbirpt.h
         third_party/icu/source/common/rbbiscan.cpp
         third_party/icu/source/common/rbbiscan.h
         third_party/icu/source/common/rbbisetb.cpp
         third_party/icu/source/common/rbbisetb.h
         third_party/icu/source/common/rbbistbl.cpp
         third_party/icu/source/common/rbbitblb.cpp
         third_party/icu/source/common/rbbitblb.h
         third_party/icu/source/common/resbund_cnv.cpp
         third_party/icu/source/common/resbund.cpp
         third_party/icu/source/common/resource.cpp
         third_party/icu/source/common/resource.h
         third_party/icu/source/common/ruleiter.cpp
         third_party/icu/source/common/ruleiter.h
         third_party/icu/source/common/schriter.cpp
         third_party/icu/source/common/serv.cpp
         third_party/icu/source/common/serv.h
         third_party/icu/source/common/servlk.cpp
         third_party/icu/source/common/servlkf.cpp
         third_party/icu/source/common/servloc.h
         third_party/icu/source/common/servls.cpp
         third_party/icu/source/common/servnotf.cpp
         third_party/icu/source/common/servnotf.h
         third_party/icu/source/common/servrbf.cpp
         third_party/icu/source/common/servslkf.cpp
         third_party/icu/source/common/sharedobject.cpp
         third_party/icu/source/common/sharedobject.h
         third_party/icu/source/common/simpleformatter.cpp
         third_party/icu/source/common/sprpimpl.h
         third_party/icu/source/common/static_unicode_sets.cpp
         third_party/icu/source/common/static_unicode_sets.h
         third_party/icu/source/common/stringpiece.cpp
         third_party/icu/source/common/stringtriebuilder.cpp
         third_party/icu/source/common/uarrsort.cpp
         third_party/icu/source/common/uarrsort.h
         third_party/icu/source/common/uassert.h
         third_party/icu/source/common/ubidi.cpp
         third_party/icu/source/common/ubidiimp.h
         third_party/icu/source/common/ubidiln.cpp
         third_party/icu/source/common/ubidi_props.cpp
         third_party/icu/source/common/ubidi_props_data.h
         third_party/icu/source/common/ubidi_props.h
         third_party/icu/source/common/ubiditransform.cpp
         third_party/icu/source/common/ubidiwrt.cpp
         third_party/icu/source/common/ubrk.cpp
         third_party/icu/source/common/ubrkimpl.h
         third_party/icu/source/common/ucase.cpp
         third_party/icu/source/common/ucase.h
         third_party/icu/source/common/ucasemap.cpp
         third_party/icu/source/common/ucasemap_imp.h
         third_party/icu/source/common/ucasemap_titlecase_brkiter.cpp
         third_party/icu/source/common/ucase_props_data.h
         third_party/icu/source/common/ucat.cpp
         third_party/icu/source/common/uchar.cpp
         third_party/icu/source/common/uchar_props_data.h
         third_party/icu/source/common/ucharstriebuilder.cpp
         third_party/icu/source/common/ucharstrie.cpp
         third_party/icu/source/common/ucharstrieiterator.cpp
         third_party/icu/source/common/uchriter.cpp
         third_party/icu/source/common/ucln_cmn.cpp
         third_party/icu/source/common/ucln_cmn.h
         third_party/icu/source/common/ucln.h
         third_party/icu/source/common/ucln_imp.h
         third_party/icu/source/common/ucmndata.cpp
         third_party/icu/source/common/ucmndata.h
         third_party/icu/source/common/ucnv2022.cpp
         third_party/icu/source/common/ucnv_bld.cpp
         third_party/icu/source/common/ucnv_bld.h
         third_party/icu/source/common/ucnvbocu.cpp
         third_party/icu/source/common/ucnv_cb.cpp
         third_party/icu/source/common/ucnv_cnv.cpp
         third_party/icu/source/common/ucnv_cnv.h
         third_party/icu/source/common/ucnv.cpp
         third_party/icu/source/common/ucnv_ct.cpp
         third_party/icu/source/common/ucnvdisp.cpp
         third_party/icu/source/common/ucnv_err.cpp
         third_party/icu/source/common/ucnv_ext.cpp
         third_party/icu/source/common/ucnv_ext.h
         third_party/icu/source/common/ucnvhz.cpp
         third_party/icu/source/common/ucnv_imp.h
         third_party/icu/source/common/ucnv_io.cpp
         third_party/icu/source/common/ucnv_io.h
         third_party/icu/source/common/ucnvisci.cpp
         third_party/icu/source/common/ucnvlat1.cpp
         third_party/icu/source/common/ucnv_lmb.cpp
         third_party/icu/source/common/ucnvmbcs.cpp
         third_party/icu/source/common/ucnvmbcs.h
         third_party/icu/source/common/ucnvscsu.cpp
         third_party/icu/source/common/ucnvsel.cpp
         third_party/icu/source/common/ucnv_set.cpp
         third_party/icu/source/common/ucnv_u16.cpp
         third_party/icu/source/common/ucnv_u32.cpp
         third_party/icu/source/common/ucnv_u7.cpp
         third_party/icu/source/common/ucnv_u8.cpp
         third_party/icu/source/common/ucol_data.h
         third_party/icu/source/common/ucol_swp.cpp
         third_party/icu/source/common/ucol_swp.h
         third_party/icu/source/common/ucptrie.cpp
         third_party/icu/source/common/ucptrie_impl.h
         third_party/icu/source/common/ucurr.cpp
         third_party/icu/source/common/ucurrimp.h
         third_party/icu/source/common/udata.cpp
         third_party/icu/source/common/udatamem.cpp
         third_party/icu/source/common/udatamem.h
         third_party/icu/source/common/udataswp.cpp
         third_party/icu/source/common/udataswp.h
         third_party/icu/source/common/uelement.h
         third_party/icu/source/common/uenum.cpp
         third_party/icu/source/common/uenumimp.h
         third_party/icu/source/common/uhash.cpp
         third_party/icu/source/common/uhash.h
         third_party/icu/source/common/uhash_us.cpp
         third_party/icu/source/common/uidna.cpp
         third_party/icu/source/common/uinit.cpp
         third_party/icu/source/common/uinvchar.cpp
         third_party/icu/source/common/uinvchar.h
         third_party/icu/source/common/uiter.cpp
         third_party/icu/source/common/ulayout_props.h
         third_party/icu/source/common/ulist.cpp
         third_party/icu/source/common/ulist.h
         third_party/icu/source/common/uloc.cpp
         third_party/icu/source/common/ulocimp.h
         third_party/icu/source/common/uloc_keytype.cpp
         third_party/icu/source/common/uloc_tag.cpp
         third_party/icu/source/common/umapfile.cpp
         third_party/icu/source/common/umapfile.h
         third_party/icu/source/common/umath.cpp
         third_party/icu/source/common/umutablecptrie.cpp
         third_party/icu/source/common/umutex.cpp
         third_party/icu/source/common/umutex.h
         third_party/icu/source/common/unames.cpp
         third_party/icu/source/common/unifiedcache.cpp
         third_party/icu/source/common/unifiedcache.h
         third_party/icu/source/common/unifilt.cpp
         third_party/icu/source/common/unifunct.cpp
         third_party/icu/source/common/uniset_closure.cpp
         third_party/icu/source/common/uniset.cpp
         third_party/icu/source/common/uniset_props.cpp
         third_party/icu/source/common/unisetspan.cpp
         third_party/icu/source/common/unisetspan.h
         third_party/icu/source/common/unistrappender.h
         third_party/icu/source/common/unistr_case.cpp
         third_party/icu/source/common/unistr_case_locale.cpp
         third_party/icu/source/common/unistr_cnv.cpp
         third_party/icu/source/common/unistr.cpp
         third_party/icu/source/common/unistr_props.cpp
         third_party/icu/source/common/unistr_titlecase_brkiter.cpp
         third_party/icu/source/common/unormcmp.cpp
         third_party/icu/source/common/unorm.cpp
         third_party/icu/source/common/unormimp.h
         third_party/icu/source/common/uobject.cpp
         third_party/icu/source/common/uposixdefs.h
         third_party/icu/source/common/uprops.cpp
         third_party/icu/source/common/uprops.h
         third_party/icu/source/common/uresbund.cpp
         third_party/icu/source/common/ures_cnv.cpp
         third_party/icu/source/common/uresdata.cpp
         third_party/icu/source/common/uresdata.h
         third_party/icu/source/common/uresimp.h
         third_party/icu/source/common/ureslocs.h
         third_party/icu/source/common/usc_impl.cpp
         third_party/icu/source/common/usc_impl.h
         third_party/icu/source/common/uscript.cpp
         third_party/icu/source/common/uscript_props.cpp
         third_party/icu/source/common/uset.cpp
         third_party/icu/source/common/uset_imp.h
         third_party/icu/source/common/usetiter.cpp
         third_party/icu/source/common/uset_props.cpp
         third_party/icu/source/common/ushape.cpp
         third_party/icu/source/common/usprep.cpp
         third_party/icu/source/common/ustack.cpp
         third_party/icu/source/common/ustrcase.cpp
         third_party/icu/source/common/ustrcase_locale.cpp
         third_party/icu/source/common/ustr_cnv.cpp
         third_party/icu/source/common/ustr_cnv.h
         third_party/icu/source/common/ustrenum.cpp
         third_party/icu/source/common/ustrenum.h
         third_party/icu/source/common/ustrfmt.cpp
         third_party/icu/source/common/ustrfmt.h
         third_party/icu/source/common/ustr_imp.h
         third_party/icu/source/common/ustring.cpp
         third_party/icu/source/common/ustr_titlecase_brkiter.cpp
         third_party/icu/source/common/ustrtrns.cpp
         third_party/icu/source/common/ustr_wcs.cpp
         third_party/icu/source/common/utext.cpp
         third_party/icu/source/common/utf_impl.cpp
         third_party/icu/source/common/util.cpp
         third_party/icu/source/common/util.h
         third_party/icu/source/common/util_props.cpp
         third_party/icu/source/common/utrace.cpp
         third_party/icu/source/common/utracimp.h
         third_party/icu/source/common/utrie2_builder.cpp
         third_party/icu/source/common/utrie2.cpp
         third_party/icu/source/common/utrie2.h
         third_party/icu/source/common/utrie2_impl.h
         third_party/icu/source/common/utrie.cpp
         third_party/icu/source/common/utrie.h
         third_party/icu/source/common/utrie_swap.cpp
         third_party/icu/source/common/uts46.cpp
         third_party/icu/source/common/utypeinfo.h
         third_party/icu/source/common/utypes.cpp
         third_party/icu/source/common/uvector.cpp
         third_party/icu/source/common/uvector.h
         third_party/icu/source/common/uvectr32.cpp
         third_party/icu/source/common/uvectr32.h
         third_party/icu/source/common/uvectr64.cpp
         third_party/icu/source/common/uvectr64.h
         third_party/icu/source/common/wintz.cpp
         third_party/icu/source/common/wintz.h)

include_directories(third_party/freetype/include/)
include_directories(third_party/freetype/src/include/)

add_definitions(-DFT_CONFIG_MODULES_H=\"freetype-custom-config/ftmodule.h\")
add_definitions(-DFT_CONFIG_OPTIONS_H=\"freetype-custom-config/ftoption.h\")
add_definitions(-DFT2_BUILD_LIBRARY)
# freetype
list(
  APPEND CORE_SRC
         # third_party/freetype/include/pstables.h
         third_party/freetype/src/include/freetype/config/ftconfig.h
         third_party/freetype/src/include/freetype/config/ftheader.h
         third_party/freetype/src/include/freetype/config/ftmodule.h
         third_party/freetype/src/include/freetype/config/ftoption.h
         third_party/freetype/src/include/freetype/config/ftstdlib.h
         third_party/freetype/src/include/freetype/freetype.h
         third_party/freetype/src/include/freetype/ftadvanc.h
         third_party/freetype/src/include/freetype/ftbbox.h
         third_party/freetype/src/include/freetype/ftbdf.h
         third_party/freetype/src/include/freetype/ftbitmap.h
         third_party/freetype/src/include/freetype/ftbzip2.h
         third_party/freetype/src/include/freetype/ftcache.h
         third_party/freetype/src/include/freetype/ftchapters.h
         third_party/freetype/src/include/freetype/ftcid.h
         third_party/freetype/src/include/freetype/ftcolor.h
         third_party/freetype/src/include/freetype/ftdriver.h
         third_party/freetype/src/include/freetype/fterrdef.h
         third_party/freetype/src/include/freetype/fterrors.h
         third_party/freetype/src/include/freetype/ftfntfmt.h
         third_party/freetype/src/include/freetype/ftgasp.h
         third_party/freetype/src/include/freetype/ftglyph.h
         third_party/freetype/src/include/freetype/ftgxval.h
         third_party/freetype/src/include/freetype/ftgzip.h
         third_party/freetype/src/include/freetype/ftimage.h
         third_party/freetype/src/include/freetype/ftincrem.h
         third_party/freetype/src/include/freetype/ftlcdfil.h
         third_party/freetype/src/include/freetype/ftlist.h
         third_party/freetype/src/include/freetype/ftlzw.h
         third_party/freetype/src/include/freetype/ftmac.h
         third_party/freetype/src/include/freetype/ftmm.h
         third_party/freetype/src/include/freetype/ftmodapi.h
         third_party/freetype/src/include/freetype/ftmoderr.h
         third_party/freetype/src/include/freetype/ftotval.h
         third_party/freetype/src/include/freetype/ftoutln.h
         third_party/freetype/src/include/freetype/ftparams.h
         third_party/freetype/src/include/freetype/ftpfr.h
         third_party/freetype/src/include/freetype/ftrender.h
         third_party/freetype/src/include/freetype/ftsizes.h
         third_party/freetype/src/include/freetype/ftsnames.h
         third_party/freetype/src/include/freetype/ftstroke.h
         third_party/freetype/src/include/freetype/ftsynth.h
         third_party/freetype/src/include/freetype/ftsystem.h
         third_party/freetype/src/include/freetype/fttrigon.h
         third_party/freetype/src/include/freetype/fttypes.h
         third_party/freetype/src/include/freetype/ftwinfnt.h
         third_party/freetype/src/include/freetype/internal/autohint.h
         third_party/freetype/src/include/freetype/internal/cffotypes.h
         third_party/freetype/src/include/freetype/internal/cfftypes.h
         third_party/freetype/src/include/freetype/internal/ftcalc.h
         third_party/freetype/src/include/freetype/internal/ftdebug.h
         third_party/freetype/src/include/freetype/internal/ftdrv.h
         third_party/freetype/src/include/freetype/internal/ftgloadr.h
         third_party/freetype/src/include/freetype/internal/fthash.h
         third_party/freetype/src/include/freetype/internal/ftmemory.h
         third_party/freetype/src/include/freetype/internal/ftobjs.h
         third_party/freetype/src/include/freetype/internal/ftpsprop.h
         third_party/freetype/src/include/freetype/internal/ftrfork.h
         third_party/freetype/src/include/freetype/internal/ftserv.h
         third_party/freetype/src/include/freetype/internal/ftstream.h
         third_party/freetype/src/include/freetype/internal/fttrace.h
         third_party/freetype/src/include/freetype/internal/ftvalid.h
         third_party/freetype/src/include/freetype/internal/internal.h
         third_party/freetype/src/include/freetype/internal/psaux.h
         third_party/freetype/src/include/freetype/internal/pshints.h
         third_party/freetype/src/include/freetype/internal/services/svbdf.h
         third_party/freetype/src/include/freetype/internal/services/svcfftl.h
         third_party/freetype/src/include/freetype/internal/services/svcid.h
         third_party/freetype/src/include/freetype/internal/services/svfntfmt.h
         third_party/freetype/src/include/freetype/internal/services/svgldict.h
         third_party/freetype/src/include/freetype/internal/services/svgxval.h
         third_party/freetype/src/include/freetype/internal/services/svkern.h
         third_party/freetype/src/include/freetype/internal/services/svmetric.h
         third_party/freetype/src/include/freetype/internal/services/svmm.h
         third_party/freetype/src/include/freetype/internal/services/svotval.h
         third_party/freetype/src/include/freetype/internal/services/svpfr.h
         third_party/freetype/src/include/freetype/internal/services/svpostnm.h
         third_party/freetype/src/include/freetype/internal/services/svprop.h
         third_party/freetype/src/include/freetype/internal/services/svpscmap.h
         third_party/freetype/src/include/freetype/internal/services/svpsinfo.h
         third_party/freetype/src/include/freetype/internal/services/svsfnt.h
         third_party/freetype/src/include/freetype/internal/services/svttcmap.h
         third_party/freetype/src/include/freetype/internal/services/svtteng.h
         third_party/freetype/src/include/freetype/internal/services/svttglyf.h
         third_party/freetype/src/include/freetype/internal/services/svwinfnt.h
         third_party/freetype/src/include/freetype/internal/sfnt.h
         third_party/freetype/src/include/freetype/internal/t1types.h
         third_party/freetype/src/include/freetype/internal/tttypes.h
         third_party/freetype/src/include/freetype/t1tables.h
         third_party/freetype/src/include/freetype/ttnameid.h
         third_party/freetype/src/include/freetype/tttables.h
         third_party/freetype/src/include/freetype/tttags.h
         third_party/freetype/src/include/ft2build.h
         third_party/freetype/src/src/base/ftbase.c
         third_party/freetype/src/src/base/ftbase.h
         third_party/freetype/src/src/base/ftbitmap.c
         third_party/freetype/src/src/base/ftdebug.c
         third_party/freetype/src/src/base/ftglyph.c
         third_party/freetype/src/src/base/ftinit.c
         third_party/freetype/src/src/base/ftmm.c
         third_party/freetype/src/src/base/ftsystem.c
         third_party/freetype/src/src/cff/cff.c
         third_party/freetype/src/src/cff/cffobjs.h
         third_party/freetype/src/src/cid/type1cid.c
         third_party/freetype/src/src/psaux/psaux.c
         third_party/freetype/src/src/pshinter/pshinter.c
         third_party/freetype/src/src/psnames/psmodule.c
         third_party/freetype/src/src/raster/raster.c
         third_party/freetype/src/src/sfnt/sfnt.c
         third_party/freetype/src/src/smooth/smooth.c
         third_party/freetype/src/src/truetype/truetype.c
         third_party/freetype/src/src/type1/type1.c)
# third_party/openjpg
set(OPENJP2_SRC
    third_party/libopenjpeg20/bio.c
    third_party/libopenjpeg20/cio.c
    third_party/libopenjpeg20/dwt.c
    third_party/libopenjpeg20/event.c
    third_party/libopenjpeg20/function_list.c
    third_party/libopenjpeg20/image.c
    third_party/libopenjpeg20/invert.c
    third_party/libopenjpeg20/j2k.c
    third_party/libopenjpeg20/jp2.c
    third_party/libopenjpeg20/mct.c
    third_party/libopenjpeg20/mqc.c
    third_party/libopenjpeg20/openjpeg.c
    third_party/libopenjpeg20/opj_clock.c
    third_party/libopenjpeg20/pi.c
    third_party/libopenjpeg20/sparse_array.c
    third_party/libopenjpeg20/sparse_array.h
    third_party/libopenjpeg20/t1.c
    third_party/libopenjpeg20/t2.c
    third_party/libopenjpeg20/tcd.c
    third_party/libopenjpeg20/tgt.c
    third_party/libopenjpeg20/thread.c)
list(APPEND CORE_SRC)

if(ENABLE_XFA)

  list(APPEND CORE_SRC fpdfsdk/cpdfsdk_xfawidget.cpp
              fpdfsdk/cpdfsdk_xfawidgethandler.cpp)
endif()
add_library(pdfium SHARED ${JPEG_SRC} ${AGG_SRC} ${OPENJP2_SRC} ${CORE_SRC}
                          ${CORE_CRT_SRC} ${CORE_XGE_SRC})

include_directories(public)
add_executable(demo demo.cpp)
target_link_libraries(demo pdfium z dl)
