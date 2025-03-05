project(bitgen)

add_executable(${PROJECT_NAME}
        bitstream/bstrGenerate/bstrGener.cpp
        bitstream/bstrGenerate/CMDLoader/CMDLoader.cpp
        bitstream/bstrGenerate/CMDLoader/CMDLoaderBase.cpp
        bitstream/bstrGenerate/FRMLoader/FRMLoaderBase.cpp
        bitstream/logWriter/xmlWriter/xmlWriter.cpp
        bitstream/memLibBstr/cktMemLibBstr/cktMemLibBstr.cpp
        bitstream/memLibBstr/cktMemLibBstr/cktMemBstr/cktMemBstr.cpp
        bitstream/memLibBstr/dfotMemLibBstr/dfotMemLibBstr.cpp
        bitstream/memLibBstr/dfotMemLibBstr/dfotMemBstr/dfotMemBstr.cpp
        bitstream/tileLibBstr/cktTileLibBstr/cktTileLibBstr.cpp
        bitstream/tileLibBstr/cktTileLibBstr/cktTileBstr/cktTileBstr.cpp
        bitstream/tileLibBstr/dfotTileLibBstr/dfotTileLibBstr.cpp
        bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/dfotTileBstr.cpp
        bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/logicInDfotTile.cpp
        bitstream/tileLibBstr/dfotTileLibBstr/dfotTileBstr/routeInDfotTile.cpp
        cil/cilBase.cpp
        cil/cilLib.cpp
        cil/loadCil.cpp
        cil/clusterLib/clusterLib.cpp
        cil/clusterLib/cluster/cluster.cpp
        cil/clusterLib/cluster/dimensionInCluster.cpp
        cil/clusterLib/cluster/linkInCluster.cpp
        cil/elementLib/elemLib.cpp
        cil/elementLib/element/Element.cpp
        cil/elementLib/element/pathInElem.cpp
        cil/elementLib/element/sramInElem.cpp
        cil/siteLib/site/cfgInSite.cpp
        cil/siteLib/site/site.cpp
        cil/tileLib/tileLib.cpp
        cil/tileLib/tile/clstInTile.cpp
        cil/tileLib/tile/tile.cpp
        cil/tileLib/tile/transInTile.cpp
        cil/transLib/transmission/transmission.cpp
        circuit/circuit.cpp
        circuit/design/design.cpp
        circuit/instLib/instLib.cpp
        circuit/instLib/inst/cfgInInst.cpp
        circuit/instLib/inst/inst.cpp
        circuit/instLib/inst/posInInst.cpp
        circuit/netLib/netLib.cpp
        circuit/netLib/net/net.cpp
        circuit/netLib/net/pinInNet.cpp
        circuit/netLib/net/pipInNet.cpp
#        circuit/xdl/xdlCktHandler.cpp
#        circuit/xml/xmlCktHandler.cpp
        main/main.cpp
        main/arguments/Args.cpp
        main/BGenApp/BGenApp.cpp
        utils/expCalc.cpp
        utils/sizeSpan.cpp
        utils/specialNets.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE . utils)

target_link_libraries(${PROJECT_NAME} PRIVATE FDE::common)

install(TARGETS ${PROJECT_NAME})