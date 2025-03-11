# Line-art Illustration of Dynamic and Specular Surfaces

> #### Author
>
> Yongjin Kim (POSTECH), Jingyi Yu (University of Delaware), Xuan Yu (University of Delaware), Seungyong Lee (POSTECH)
>
> #### Paper
>
> [Paper Link](https://dl.acm.org/doi/10.1145/1409060.1409109)
>
> ACM Transactions on Graphics, 2008.

## Current Structure

> Section 5. Stroke Direction Propagation은 구현하지 않음.

Programs & Texture Infos

1. `normalPositionProgram`: 추후 사용될 여러 데이터 텍스쳐 생성. 각각에 서브루틴 1~4로 총 4회 렌더링.

   1. `dataTexture[NORMAL]`, `dataTexture[POSITION]`: Principal Direction 추정을 위해 저장.

   2. `dataTexture[PHONG]`: Edge Detection에서 Sobel Kernel이 적용될 텍스쳐.

   3. `dataTexture[EDGE]`: Edge Detection 결과를 저장.

2. `pdProgram`: Principal Direction 추정. 결과를 `pdTexture`로 내보냄.

3. `usProgram`: `pdProgram`에서 해결하지 못한 Umbilic Case 해결. 결과를 `usTexture`로 내보냄.

4. `sdProgram`: Smoothing and TAM Matching

   1. Stroke Direction Smoothing을 2회 진행. 결과를 `sdTexture`로 내보냄.

      1. `dataTexture[EDGE]`를 받아 Smoothing 경계로 사용.

      2. 내부적으로는 2회 이상도 Smoothing할 수 있도록 구현됨. 텍스쳐는 2장 사용하여 핑퐁.

   2. Stroke Direction을 X축에 대한 각으로 변환 후 TAM 매칭.

5. `quadProgram`: 최종적으로 렌더링된 텍스쳐를

## 4. Principal Direction Estimation

Purpose: 실시간으로 동적인 오브젝트의 principal directions를 이미지 스페이스에서 추정하는 알고리즘 구현.

모델의 normal과 world position을 사전에 텍스쳐로 맵핑.

해당 텍스쳐에서 세 값을 가져와 _Normal-Ray Surface Model_ 적용.

> Normal-Ray Surface Model
>
> 한 normal 값을 Z 방향으로 설정한 local frame 구성.
>
> $z=0$ plane과 $z=1$ plane에서의 세 normal의 교점 좌표를 구함.
>
> 교점이 $[u,v,0]$, $[s,t,1]$라고 하면, 해당 normal을 다음과 같이 파라미터화 한다.
>
> each normal: $[\sigma, \tau, u, v] = [s-u, t-v, u, v]$

해당 광선이 $\lambda$만큼 이동하였을 때, 세 광선은 한 슬릿에서 만나게 된다.

이 경우 세 점이 이루는 너비가 0임을 이용하여 $\lambda$를 구하면, 해당 값은 2차 방정식의 근으로 나타낼 수 있다.

해당 값은 곡률에 대응하고 두 슬릿의 방향은 principal direction과 일치한다.

두 min/max principal direction 중 min principal direction이 stroke의 방향이 된다.

추가로 표면이 locally planar, locally near parabolic, umbilic이 되는 degeneracies를 논문의 방법으로 해결한다.

## 6. Image-space Stroke Mapping

스무딩 공식은 아래와 같다. 디테일은 논문 참고.

$t'(x)=∑_{y\in\Omega_{r(x)}}ø(x,y)w_s(x,y)w_d(x,y)t(y)$

스무딩 과정은 Image space에서 이루어지므로 discontinuity를 찾아 해당 부분에서는 Smoothing을 중단한다.

이렇게 나온 direction과 X축의 사잇각 $\theta$을 구한다.

해당 각은 12 레벨로 양자화된다. 이를 $\theta_k$라 하자.

인근한 양자화 레벨 $\theta_{k-1},\theta_{k+1}$과 $\theta_k$의 세 각을 $\theta$와의 차이로 가중하여 blending을 진행한다.

이렇게 blending된 각만큼 회전된 좌표를 사용하여 TAM의 텍스쳐를 불러온다.

$T(x,y)=(cos\theta_k\cdot x-\sin\theta_k\cdot y, \sin\theta_k\cdot x+\cos\theta_k\cdot y)$

끝!
