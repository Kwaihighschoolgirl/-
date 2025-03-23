document.addEventListener('DOMContentLoaded', function() {
    // 네비게이션 스크롤 이벤트
    const nav = document.querySelector('.nav-wrapper');
    
    window.addEventListener('scroll', function() {
        if (window.scrollY > 50) {
            nav.style.padding = '8px 0';
            nav.style.boxShadow = '0 2px 10px rgba(0,0,0,0.1)';
        } else {
            nav.style.padding = '16px 0';
            nav.style.boxShadow = 'none';
        }
    });
    
    // 스무스 스크롤 이벤트
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            
            const targetId = this.getAttribute('href');
            const targetElement = document.querySelector(targetId);
            
            if (targetElement) {
                window.scrollTo({
                    top: targetElement.offsetTop - 80,
                    behavior: 'smooth'
                });
            }
        });
    });
    
    // 섹션 애니메이션
    const sections = document.querySelectorAll('.section');
    
    function checkSections() {
        const triggerBottom = (window.innerHeight / 5) * 4;
        
        sections.forEach(section => {
            const sectionTop = section.getBoundingClientRect().top;
            
            if (sectionTop < triggerBottom) {
                section.style.opacity = 1;
                section.style.transform = 'translateY(0)';
            }
        });
    }
    
    // 초기 스타일 설정
    sections.forEach(section => {
        section.style.opacity = 0;
        section.style.transform = 'translateY(30px)';
        section.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
    });
    
    // 페이지 로드 시 한 번 확인
    checkSections();
    
    // 스크롤 시 확인
    window.addEventListener('scroll', checkSections);
    
    // 카드 hover 효과 보강
    const cards = document.querySelectorAll('.card');
    
    cards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-10px)';
            this.style.boxShadow = '0 15px 40px rgba(0, 0, 0, 0.15)';
            
            const image = this.querySelector('.card-image img');
            if (image) {
                image.style.transform = 'scale(1.05)';
            }
        });
        
        card.addEventListener('mouseleave', function() {
            this.style.transform = 'translateY(0)';
            this.style.boxShadow = '0 10px 30px rgba(0, 0, 0, 0.1)';
            
            const image = this.querySelector('.card-image img');
            if (image) {
                image.style.transform = 'scale(1)';
            }
        });
    });
}); 