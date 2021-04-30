pgd_t *pgd;
p4d_t *p4d;
pmd_t *pmd;
pud_t *pud;
pte_t *ptep, pte;

struct page *page = NULL;
struct mm_struct *mm = current->mm;

pgd = pgd_offset(mm, address);                    // get pgd from mm and the page address
if (pgd_none(*pgd) || pgd_bad(*pgd)){           // check if pgd is bad or does not exist
    return page;
}else{

}

p4d = p4d_offset(pgd, address);                   // get p4d from from pgd and the page address
if (p4d_none(*p4d) || p4d_bad(*p4d)){          // check if p4d is bad or does not exist
    return page;
} else{

}

pud = pud_offset(p4d, address);                   // get pud from from p4d and the page address
if (pud_none(*pud) || pud_bad(*pud)){          // check if pud is bad or does not exist
    return page;
} else{

}

pmd = pmd_offset(pud, address);               // get pmd from from pud and the page address
if (pmd_none(*pmd) || pmd_bad(*pmd)){	   // check if pmd is bad or does not exist
    return page;
} else{

}

ptep = pte_offset_map(pmd, address);      // get pte from pmd and the page address
if (!ptep){
    return page;
}                                         // check if pte does not exist
pte = *ptep;

page = pte_page(pte);
if(page){
    // print [PID]:[WSS]
    printk(page);
}
pte_unmap(ptep);
