# OpenGL_URDFviewer

개발환경<br/>
C++<br/>
Visual Studio 2022

사용 라이브러리<br/>
glad<br/>
GLFW<br/>
glm<br/>
tinyxml2<br/>
Eigen<br/>
stb<br/>
khr<br/>


졸업작품때 제작한 로봇 팔을 Solidworks에서 URDF로 변환하였다. 하지만 urdf 파일은 보통 리눅스에서 ROS를 위한 파일 형식으로, 윈도우에서 열 수 있는 프로그램이 있으면 어떨까 하고 제작하였다.

기본적인 OpenGL의 사용법은 해당 유튜브 강의와 깃헙을 공부하였다.

https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-

https://github.com/VictorGordan/opengl-tutorials

해당 튜토리얼을 따라하면서 쉐이더의 기초 내용과 렌더링 파이프라인, OpenGL의 기본적인 사용법에 대해 숙지하고 원하는 기능을 만드는 걸 목표로 하였다.

URDF파일을 읽기 위해서 tinyxml을 사용해 xml 파일을 읽고, STL, 조인트 정보 등을 불러와서 OpenGL 상에서 나타내는 기능을 구현하였다.

실행하면 새 창이 하나 나온다. SOLIDWORKS로 제작한 ROS용 패키지 속 URDF파일을 집어넣으면 URDF모델을 출력한다.

![urdfviewer](https://github.com/user-attachments/assets/fb04d410-754d-4c30-9a40-2943e1ba6996)



개선할 점

먼저 카메라가 생각보다 불편해서 더 개선을 해야겠다.

또한 조인트의 입력에 따라 각 링크가 얼만큼 회전하는지 애니메이션을 넣으면 좋겠다는 생각을 했다.

하지만 현재로서는 Main파일의 SetURDF 함수에서 모든 링크의 vertex들을 하나의 vertices에 넣어서 출력을 했는데, 애니메이션을 하려면 링크의 갯수만큼 vertices를 생성해서 회전을 시켜줘야 하겠다. 훗날 OpenGL을 더 사용할 일이 생기면 더 공부해서 진행해 보아야 겠다.
