# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
# Using absolute paths to prevent improper interpretation of relative paths. Relative paths can be used once the minimum
# CMake version is greater or equal than CMake 3.13
target_sources(rmlui_core PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/AreaGraph.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/BaseGraph.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CandleBarGraph.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CandleGraph.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataFeed.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataFeedAggregate.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataFeedConverter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataFeedTransforms.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataFeedTypeCheck.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GraphGrid.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GraphScale.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LineGraph.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/MakeFeed.cpp"
)
