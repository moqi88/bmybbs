#include <stdio.h>
#include "sectree.h"
const struct sectree sectree;
static const struct sectree sectreeA;
static const struct sectree sectreeAA;
static const struct sectree sectreeB;
static const struct sectree sectreeBA;
static const struct sectree sectreeBB;
static const struct sectree sectreeC;
static const struct sectree sectreeD;
static const struct sectree sectreeE;
static const struct sectree sectreeF;
static const struct sectree sectreeG;
static const struct sectree sectreeH;
static const struct sectree sectreeI;
static const struct sectree sectreeIA;
static const struct sectree sectreeJ;
static const struct sectree sectreeJA;
static const struct sectree sectreeK;
static const struct sectree sectreeL;
static const struct sectree sectreeLA;
static const struct sectree sectreeM;
static const struct sectree sectreeN;
static const struct sectree sectreeO;
/*------ sectree -------*/
const struct sectree sectree = {
	parent: NULL,
	title: "����ٸBBS",
	basestr: "",
	seccodes: "0123456789GNHAC",
	introstr: "0123456789GNHCA",
	des: "",
	nsubsec: 15,
	subsec: {
		&sectreeA,
		&sectreeB,
		&sectreeC,
		&sectreeD,
		&sectreeE,
		&sectreeF,
		&sectreeG,
		&sectreeH,
		&sectreeI,
		&sectreeJ,
		&sectreeK,
		&sectreeL,
		&sectreeM,
		&sectreeN,
		&sectreeO,
	}
};
/*------ sectreeA -------*/
static const struct sectree sectreeA = {
	parent: &sectree,
	title: "��վϵͳ",
	basestr: "0",
	seccodes: "BC",
	introstr: "BC",
	des: "[��վ]",
	nsubsec: 1,
	subsec: {
		&sectreeAA,
	}
};
/*------ sectreeAA -------*/
static const struct sectree sectreeAA = {
	parent: &sectreeA,
	title: "��������",
	basestr: "0B",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
static const struct sectree sectreeAC = {
	parent: &sectreeA,
	title: "�ѹذ���",
	basestr: "0C",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeB -------*/
static const struct sectree sectreeB = {
	parent: &sectree,
	title: "��ͨ��ѧ",
	basestr: "1",
	seccodes: "AB",
	introstr: "ABC",
	des: "[Ժϵ]",
	nsubsec: 2,
	subsec: {
		&sectreeBA,
		&sectreeBB,
	}
};
/*------ sectreeBA -------*/
static const struct sectree sectreeBA = {
	parent: &sectreeB,
	title: "Ժϵ",
	basestr: "1A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeBB -------*/
static const struct sectree sectreeBB = {
	parent: &sectreeB,
	title: "����Ⱥ��",
	basestr: "1B",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeC -------*/
static const struct sectree sectreeC = {
	parent: &sectree,
	title: "��������",
	basestr: "2",
	seccodes: "",
	introstr: "",
	des: "[���][����]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeD -------*/
static const struct sectree sectreeD = {
	parent: &sectree,
	title: "����Ӧ��",
	basestr: "3",
	seccodes: "",
	introstr: "",
	des: "[����][Ӧ��]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeE -------*/
static const struct sectree sectreeE = {
	parent: &sectree,
	title: "ѧ����ѧ",
	basestr: "4",
	seccodes: "",
	introstr: "",
	des: "[ѧ��][��ѧ]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeF -------*/
static const struct sectree sectreeF = {
	parent: &sectree,
	title: "����ѧ",
	basestr: "5",
	seccodes: "",
	introstr: "",
	des: "[����][���]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeG -------*/
static const struct sectree sectreeG = {
	parent: &sectree,
	title: "��ѧ����",
	basestr: "6",
	seccodes: "",
	introstr: "",
	des: "[��ѧ][����]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeH -------*/
static const struct sectree sectreeH = {
	parent: &sectree,
	title: "֪�Ը���",
	basestr: "7",
	seccodes: "",
	introstr: "",
	des: "[����][����]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeI -------*/
static const struct sectree sectreeI = {
	parent: &sectree,
	title: "�����˶�",
	basestr: "8",
	seccodes: "A",
	introstr: "A",
	des: "[����]",
	nsubsec: 1,
	subsec: {
		&sectreeIA,
	}
};
/*------ sectreeIA -------*/
static const struct sectree sectreeIA = {
	parent: &sectreeI,
	title: "������ֲ�",
	basestr: "8A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeJ -------*/
static const struct sectree sectreeJ = {
	parent: &sectree,
	title: "��������",
	basestr: "9",
	seccodes: "A",
	introstr: "A",
	des: "[����][����]",
	nsubsec: 1,
	subsec: {
		&sectreeJA,
	}
};
/*------ sectreeJA -------*/
static const struct sectree sectreeJA = {
	parent: &sectreeJ,
	title: "���Ǿ��ֲ�",
	basestr: "9A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeK -------*/
static const struct sectree sectreeK = {
	parent: &sectree,
	title: "��Ϸ���",
	basestr: "G",
	seccodes: "",
	introstr: "",
	des: "[��Ϸ]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeL -------*/
static const struct sectree sectreeL = {
	parent: &sectree,
	title: "������Ϣ",
	basestr: "N",
	seccodes: "B",
	introstr: "B",
	des: "[����][��Ϣ]",
	nsubsec: 1,
	subsec: {
		&sectreeLA,
	}
};
/*------ sectreeLA -------*/
static const struct sectree sectreeLA = {
	parent: &sectreeL,
	title: "�����г�",
	basestr: "NB",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeM -------*/
static const struct sectree sectreeM = {
	parent: &sectree,
	title: "��������",
	basestr: "H",
	seccodes: "",
	introstr: "",
	des: "[����][����]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeN -------*/
static const struct sectree sectreeN = {
	parent: &sectree,
	title: "У����Ϣ",
	basestr: "A",
	seccodes: "",
	introstr: "",
	des: "[У��][��Ϣ]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeO -------*/
static const struct sectree sectreeO = {
	parent: &sectree,
	title: "���ֲ���",
	basestr: "C",
	seccodes: "",
	introstr: "",
	des: "     [���ֲ�]",
	nsubsec: 0,
	subsec: {
	}
};
