# webserv42
webserv of heryu hyeonkim mijeong


1. dev 브랜치 땡겨오기
  git pull origin dev
2. 작업할 로컬 브랜치 만들기
  git checkout -b "새로운 브랜치 이름"
3. 작업 완료된 다음에 "새로운 브랜치 이름에 push 하기"
  git add (수정한 파일)
  git commit -m (작업내용 파일)
  git push -u origin "새로운 브랜치 이름"
4. github 페이지에서 pull request 날리기
  base:dev, compare:"새로운 브랜치 이름"
