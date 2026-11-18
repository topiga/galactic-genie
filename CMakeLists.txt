cmake_minimum_required(VERSION 3.16)

# Vita:  cmake -B build-vita -G Ninja -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
# PC:    cmake -B build-pc -G Ninja          (builds swgbtool, the headless data/render tool)
project(swgb_vita C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SWGB_CORE_SOURCES
    src/core/audio.cpp
    src/core/blendomatic.cpp
    src/core/cpx.cpp
    src/core/drs.cpp
    src/core/fog_edges.cpp
    src/core/elevation.cpp
    src/core/palette.cpp
    src/core/scenario.cpp
    src/core/slope_lighting.cpp
    src/core/slp.cpp
    src/core/genie_dat.cpp
    src/core/language_strings.cpp
    src/engine/assets.cpp
    src/engine/ai_script.cpp
    src/engine/rms_script.cpp
    src/engine/rms_scenery.cpp
    src/engine/rms_early_forest.cpp
    src/engine/rms_scenery_appearance.cpp
    src/engine/rms_objects.cpp
    src/engine/rms_object_rows.cpp
    src/engine/rms_terrain_cleanup.cpp
    src/engine/rms_terrain.cpp
    src/engine/rms_elevation.cpp
    src/engine/rms_cliffs.cpp
    src/engine/rms_land_placement.cpp
    src/engine/rms_connections.cpp
    src/engine/rms_slopes.cpp
    src/engine/rms_zones.cpp
    src/engine/rms_walls.cpp
    src/engine/rms_map.cpp
    src/engine/rms_player_starts.cpp
    src/engine/ai_military.cpp
    src/engine/ai_targets.cpp
    src/engine/ai_group_targets.cpp
    src/engine/game.cpp
    src/engine/game_tactical_movement.cpp
    src/engine/game_military.cpp
    src/engine/game_explore_destination.cpp
    src/engine/game_ai_economy.cpp
    src/engine/game_ai_build_placement.cpp
    src/engine/game_ai_civilian_explore.cpp
    src/engine/game_ai_context.cpp
    src/engine/game_power.cpp
    src/engine/game_ai_resources.cpp
    src/engine/game_ai_memory.cpp
    src/engine/pathfinding.cpp
)

if(VITA)
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

    set(VITA_APP_NAME "SWGB Vita")
    set(VITA_TITLEID "SWGB00001")
    set(VITA_VERSION "00.01")
    option(SWGB_VPK_LIVEAREA "Bundle LiveArea icon/background into the VPK" ON)

    add_executable(swgb
        ${SWGB_CORE_SOURCES}
        src/render/gl_renderer.cpp
        src/platform/vita/vita_audio.cpp
        src/platform/vita/main_vita.cpp
    )
    target_compile_options(swgb PRIVATE
        -Os -ffunction-sections -fdata-sections
        -Wall -Wno-psabi)
    # -Wl,-q keeps relocations; vita-elf-create needs them for a valid velf.
    target_link_options(swgb PRIVATE -Wl,-q,--gc-sections)
    target_link_libraries(swgb PRIVATE
        vitaGL vitashark SceShaccCgExt taihen_stub mathneon z
        SceGxm_stub SceDisplay_stub SceCtrl_stub SceTouch_stub SceHid_stub
        SceAudio_stub SceCommonDialog_stub SceSysmodule_stub SceMotion_stub
        SceAppUtil_stub SceAppMgr_stub SceShaccCg_stub ScePower_stub
        SceKernelDmacMgr_stub SceIme_stub
        vorbisfile vorbis ogg stdc++ m)

    # 32MB phycont budget: without it vitaGL renders nothing on this setup.
    set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -pm 0x2000000")
    # UNSAFE = full permissions, needed for the 256MB newlib heap.
    vita_create_self(eboot.bin swgb UNSAFE)

    if(SWGB_VPK_LIVEAREA)
        vita_create_vpk(swgb.vpk ${VITA_TITLEID} eboot.bin
            VERSION ${VITA_VERSION}
            NAME ${VITA_APP_NAME}
            FILE vita/sce_sys/icon0.png sce_sys/icon0.png
                 vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
                 vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
                 vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        )
    else()
        vita_create_vpk(swgb.vpk ${VITA_TITLEID} eboot.bin VERSION ${VITA_VERSION} NAME ${VITA_APP_NAME})
    endif()
else()
    find_package(ZLIB REQUIRED)
    add_executable(swgbtool
        tools/swgbtool.cpp
        ${SWGB_CORE_SOURCES}
        src/render/soft_renderer.cpp
    )
    target_link_libraries(swgbtool PRIVATE ZLIB::ZLIB)
    add_executable(swgb-rms-snapshot EXCLUDE_FROM_ALL tools/rms_snapshot.cpp
        ${SWGB_CORE_SOURCES} src/render/soft_renderer.cpp)
    target_link_libraries(swgb-rms-snapshot PRIVATE ZLIB::ZLIB)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(swgbtool PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
    endif()
endif()

if(NOT VITA)
    option(SWGB_BUILD_DESKTOP "Build the SDL desktop frontend" OFF)
    if(SWGB_BUILD_DESKTOP)
        find_package(SDL2 REQUIRED)
        find_package(Threads REQUIRED)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(VORBISFILE REQUIRED IMPORTED_TARGET vorbisfile)
        add_executable(swgb-desktop src/platform/desktop/main_sdl.cpp
            ${SWGB_CORE_SOURCES} src/render/soft_renderer.cpp)
        target_link_libraries(swgb-desktop PRIVATE ZLIB::ZLIB SDL2::SDL2 PkgConfig::VORBISFILE Threads::Threads)
        if(APPLE)
            target_link_libraries(swgb-desktop PRIVATE "-framework CoreText" "-framework CoreGraphics" "-framework CoreFoundation")
        endif()
    endif()
endif()

if(NOT VITA)
    option(SWGB_BUILD_SKIRMISH_TEST "Build the asset-dependent skirmish test" OFF)
    if(SWGB_BUILD_SKIRMISH_TEST)
        add_executable(swgb-skirmish-test tools/skirmish_test.cpp
            ${SWGB_CORE_SOURCES} src/render/soft_renderer.cpp)
        target_link_libraries(swgb-skirmish-test PRIVATE ZLIB::ZLIB)
    endif()
endif()

if(NOT VITA)
    option(SWGB_BUILD_ALGORITHM_TESTS "Build asset-independent RMS and military tests" OFF)
    if(SWGB_BUILD_ALGORITHM_TESTS)
        enable_testing()
        add_executable(swgb-rms-test tools/rms_test.cpp src/engine/rms_script.cpp src/core/drs.cpp)
        add_executable(swgb-rms-object-rows-test tools/rms_object_rows_test.cpp src/engine/rms_object_rows.cpp)
        add_executable(swgb-rms-object-test tools/rms_object_test.cpp src/engine/rms_zones.cpp src/engine/rms_objects.cpp src/engine/rms_scenery_appearance.cpp)
        add_executable(swgb-ai-military-test tools/ai_military_test.cpp src/engine/ai_military.cpp)
        add_executable(swgb-ai-targets-test tools/ai_targets_test.cpp src/engine/ai_targets.cpp)
        add_executable(swgb-ai-group-targets-test tools/ai_group_targets_test.cpp src/engine/ai_group_targets.cpp src/engine/ai_targets.cpp)
        add_executable(swgb-ai-group-search-test tools/ai_group_search_test.cpp)
        add_executable(swgb-ai-attack-memory-test tools/ai_attack_memory_test.cpp)
        add_executable(swgb-ai-player-facts-test tools/ai_player_facts_test.cpp)
        add_executable(swgb-ai-resource-knowledge-test tools/ai_resource_knowledge_test.cpp)
        add_executable(swgb-ai-session-context-test tools/ai_session_context_test.cpp)
        add_executable(swgb-ai-build-admission-test tools/ai_build_admission_test.cpp)
        add_executable(swgb-ai-gather-allocation-test tools/ai_gather_allocation_test.cpp)
        add_executable(swgb-ai-gather-quota-test tools/ai_gather_quota_test.cpp)
        add_executable(swgb-ai-resource-selection-test tools/ai_resource_selection_test.cpp)
        add_executable(swgb-ai-resource-lifecycle-test tools/ai_resource_lifecycle_test.cpp)
        add_executable(swgb-ai-economic-delivery-test tools/ai_economic_delivery_test.cpp)
        add_executable(swgb-ai-gather-controller-test tools/ai_gather_controller_test.cpp)
        add_executable(swgb-ai-build-placement-test tools/ai_build_placement_test.cpp)
        add_executable(swgb-ai-builder-selection-test tools/ai_builder_selection_test.cpp)
        add_executable(swgb-ai-prefab-grid-test tools/ai_prefab_grid_test.cpp)


        add_executable(swgb-rms-map-test tools/rms_map_test.cpp src/engine/rms_walls.cpp src/engine/rms_land_placement.cpp src/engine/rms_zones.cpp src/engine/rms_connections.cpp src/engine/rms_elevation.cpp src/engine/rms_cliffs.cpp src/engine/rms_slopes.cpp src/engine/rms_early_forest.cpp src/engine/rms_terrain_cleanup.cpp src/engine/rms_scenery.cpp src/engine/rms_scenery_appearance.cpp src/engine/rms_terrain.cpp src/engine/rms_map.cpp src/engine/rms_object_rows.cpp src/engine/rms_script.cpp src/engine/rms_objects.cpp src/engine/rms_player_starts.cpp src/core/drs.cpp)
        add_executable(swgb-rms-player-starts-test tools/rms_player_starts_test.cpp src/engine/rms_player_starts.cpp)
        add_executable(swgb-rms-cliffs-test tools/rms_cliffs_test.cpp src/engine/rms_cliffs.cpp)
        add_executable(swgb-rms-elevation-test tools/rms_elevation_test.cpp src/engine/rms_elevation.cpp src/engine/rms_slopes.cpp)
        add_executable(swgb-rms-land-placement-test tools/rms_land_placement_test.cpp src/engine/rms_land_placement.cpp)
        add_executable(swgb-rms-connections-test tools/rms_connections_test.cpp src/engine/rms_connections.cpp src/engine/rms_script.cpp src/engine/rms_terrain_cleanup.cpp src/core/drs.cpp)
        add_executable(swgb-rms-slopes-test tools/rms_slopes_test.cpp src/engine/rms_slopes.cpp)
        add_executable(swgb-rms-terrain-test tools/rms_terrain_test.cpp src/engine/rms_terrain.cpp)
        add_executable(swgb-rms-scenery-test tools/rms_scenery_test.cpp src/engine/rms_scenery.cpp src/engine/rms_scenery_appearance.cpp)
        add_executable(swgb-rms-terrain-cleanup-test tools/rms_terrain_cleanup_test.cpp src/engine/rms_terrain_cleanup.cpp)
        add_executable(swgb-rms-scenery-appearance-test tools/rms_scenery_appearance_test.cpp src/engine/rms_scenery_appearance.cpp)
        add_executable(swgb-rms-early-forest-test tools/rms_early_forest_test.cpp src/engine/rms_early_forest.cpp src/engine/rms_scenery.cpp src/engine/rms_scenery_appearance.cpp)
        add_executable(swgb-rms-group-test tools/rms_group_test.cpp src/engine/rms_zones.cpp src/engine/rms_objects.cpp src/engine/rms_scenery_appearance.cpp)
        add_executable(swgb-rms-zones-test tools/rms_zones_test.cpp src/engine/rms_zones.cpp)
        foreach(test rms rms-object-rows rms-cliffs rms-land-placement rms-connections rms-elevation rms-slopes rms-zones rms-group rms-object rms-map rms-player-starts rms-terrain rms-scenery rms-scenery-appearance rms-early-forest rms-terrain-cleanup ai-military ai-targets ai-group-targets ai-group-search ai-attack-memory ai-player-facts ai-resource-knowledge ai-session-context ai-build-admission ai-gather-allocation ai-gather-quota ai-resource-selection ai-resource-lifecycle ai-economic-delivery ai-gather-controller ai-build-placement ai-builder-selection ai-prefab-grid)
            target_include_directories(swgb-${test}-test PRIVATE src)
            add_test(NAME ${test} COMMAND swgb-${test}-test)
        endforeach()
    endif()
endif()
