CXX = g++

# 빌드 옵션 분리
CXXFLAGS_RELEASE = -Wall -O2 -std=c++17
LDFLAGS_RELEASE  =

CXXFLAGS_DEBUG   = -Wall -O1 -std=c++17 -g -fsanitize=address
LDFLAGS_DEBUG    = -fsanitize=address

# 기본 타겟: release
all: release

# release 빌드
release: CXXFLAGS = $(CXXFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: evaltsp basic_HK basic_CH

# debug 빌드 (make db)
db: CXXFLAGS = $(CXXFLAGS_DEBUG)
db: LDFLAGS = $(LDFLAGS_DEBUG)
db: evaltsp basic_HK basic_CH

# 실행 파일 빌드
evaltsp: evaltsp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

basic_HK: basic_HK.o
	$(CXX) -o $@ $^ $(LDFLAGS)

basic_CH: basic_CH.o
	$(CXX) -o $@ $^ $(LDFLAGS)

# 컴파일
%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# 정리
clean:
	rm -f evaltsp basic_HK basic_CH *.o
