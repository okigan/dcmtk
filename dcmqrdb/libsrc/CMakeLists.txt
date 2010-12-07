# create library from source files
ADD_LIBRARY(dcmqrdb dcmqrcbf dcmqrcbg dcmqrcbm dcmqrcbs dcmqrcnf dcmqrdbi dcmqrdbq dcmqrdbs dcmqropt dcmqrptb dcmqrsrv dcmqrtis)

if (WITH_SQL_DATABASE)
  SET(LIBS dblib)
  
  INCLUDE_DIRECTORIES(${dblib_SOURCE_DIR}/include )
endif (WITH_SQL_DATABASE)

TARGET_LINK_LIBRARIES(dcmqrdb ${LIBS})

# declare installation files
INSTALL_TARGETS(${INSTALL_LIBDIR} dcmqrdb)
