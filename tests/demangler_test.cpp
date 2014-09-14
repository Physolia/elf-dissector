#include <QtTest/qtest.h>
#include <QObject>

#include "../demangle/demangler.cpp"

#define VB QVector<QByteArray>()

class DemanglerTest : public QObject
{
    Q_OBJECT
private slots:
    void testDemangler_data()
    {
        QTest::addColumn<QString>("mangled"); // should be QByteArray, but then we have to explicitly cast it below
        QTest::addColumn<QVector<QByteArray>>("expectedDemangled");

        QTest::newRow("empty") << "" << (VB << "");
        QTest::newRow("C 1") << "malloc" << (VB << "malloc");
        QTest::newRow("member func 1") << "_ZN10QArrayData4dataEv" << (VB << "QArrayData" << "data()");
        QTest::newRow("member func 2") << "_ZN10QByteArray6appendERKS_" << (VB << "QByteArray" << "append(QByteArray const&)");
        QTest::newRow("member func 3") << "_ZN10QByteArray6numberEii" << (VB << "QByteArray" << "number(int, int)");
        QTest::newRow("ctor 1") << "_ZN10QByteArrayC1EPKci" << (VB << "QByteArray" << "QByteArray(char const*, int)");
        QTest::newRow("copy ctor 1") << "_ZN10QByteArrayC1ERKS_" << (VB << "QByteArray" << "QByteArray(QByteArray const&)");
        QTest::newRow("copy ctor 2") << "_ZN10QByteArrayC2ERKS_" << (VB << "QByteArray" << "QByteArray(QByteArray const&)");
        QTest::newRow("dtor 1") << "_ZN10QByteArrayD1Ev" << (VB << "QByteArray" << "~QByteArray()");
        QTest::newRow("dtor 2") << "_ZN10QByteArrayD2Ev" << (VB << "QByteArray" << "~QByteArray()");
        QTest::newRow("operator 1") << "_ZN10QByteArraypLERKS_" << (VB << "QByteArray" << "operator+=(QByteArray const&)");
        QTest::newRow("operator 2") << "_ZNK10QByteArraycvPKcEv" << (VB << "QByteArray" << "operator char const*() const");

        QTest::newRow("template func 1") <<  "_Z13qGetPtrHelperI14QScopedPointerI11QObjectData21QScopedPointerDeleterIS1_EEENT_7pointerERKS5_" << (VB << "qGetPtrHelper" << "qGetPtrHelper<QScopedPointer<QObjectData, QScopedPointerDeleter<QObjectData>>>(QScopedPointer<QObjectData, QScopedPointerDeleter<QObjectData>> const&)");
        QTest::newRow("template func 2") << "_ZN23QXmlStreamWriterPrivate5writeILi4EEEvRAT__Kc" << (VB << "QXmlStreamWriterPrivate" << "write" << "write<4>(char const (&) [4])");

        QTest::newRow("function pointer template") << "_ZN20QGlobalStaticDeleterI5QListIPFP7QObjectvEEED1Ev" << (VB << "QGlobalStaticDeleter" << "QGlobalStaticDeleter<QList<QObject* (*)()>>" << "~QGlobalStaticDeleter()");

        QTest::newRow("lambda 1") << "_ZSt7find_ifIPKSt10shared_ptrI7ElfFileEZN10ElfFileSet7addFileERK7QStringEUlRS3_E_ET_SB_SB_T0_" << (VB << "std" << "find_if" << "find_if<std::shared_ptr<ElfFile> const*, ElfFileSet::addFile(QString const&)::{lambda(std::shared_ptr<ElfFile> const&)#1}>(ElfFileSet::addFile(QString const&)::{lambda(std::shared_ptr<ElfFile> const&)#1}, ElfFileSet::addFile(QString const&)::{lambda(std::shared_ptr<ElfFile> const&)#1}, ElfFileSet::addFile(QString const&)::{lambda(std::shared_ptr<ElfFile> const&)#1})");
        QTest::newRow("lambda 2") << "_ZZN13Ui_MainWindow7setupUiEP11QMainWindowENKUlvE5_clEv" << (VB << "Ui_MainWindow" << "setupUi(QMainWindow*)" << "{lambda()#7}" << "operator()() const");

        QTest::newRow("tmp 1") << "_Z27qRegisterNormalizedMetaTypeI7QVectorI10QByteArrayEEiRKS1_PT_N9QtPrivate21MetaTypeDefinedHelperIS5_Xaasr12QMetaTypeId2IS5_E7DefinedntsrSA_9IsBuiltInEE11DefinedTypeE" << (VB << "qRegisterNormalizedMetaType" << /*"qRegisterNormalizedMetaType<QVector<QByteArray>>(QByteArray const&, QVector<QByteArray>*, QtPrivate::MetaTypeDefinedHelper<QVector<QByteArray>, QMetaTypeId2<QVector<QByteArray>>::Defined&&(!QMetaTypeId2<QVector<QByteArray>>::IsBuiltIn)>::DefinedType)"*/ "qRegisterNormalizedMetaType<QVector<QByteArray>>(QByteArray const&, QVector<QByteArray>*, QtPrivate::MetaTypeDefinedHelper<QVector<QByteArray>, QMetaTypeId2<QVector<QByteArray>>::Defined&&!QMetaTypeId2<QVector<QByteArray>>::IsBuiltIn>::DefinedType)"); // TODO find a way to fix the parenthesis in unary/binary expressions

        QTest::newRow("pack 1") << "_ZSt12__get_helperILm0EPN2Ui10MainWindowEISt14default_deleteIS1_EEENSt9__add_refIT0_E4typeERSt11_Tuple_implIXT_EIS6_DpT1_EE" << (VB << "std" << "__get_helper" << "__get_helper<0ul, Ui::MainWindow*, std::default_delete<Ui::MainWindow>>(std::_Tuple_impl<0ul, Ui::MainWindow*, std::default_delete<Ui::MainWindow>>&)");

        QTest::newRow("vendor 1") << "_ZL18mergeQuestionMarksU8__vectorx" << (VB << "mergeQuestionMarks(long long __vector)");

        QTest::newRow("vector type") << "_ZN9QSimdSse25v_mulEDv4_fS0_" << (VB << "QSimdSse2" << "v_mul(float __vector(4), float __vector(4))");

        QTest::newRow("decltype in return") << "_ZNSt16allocator_traitsISaIiEE9constructIiJRKiEEEDTcl12_S_constructfp_fp0_spcl7forwardIT0_Efp1_EEERS0_PT_DpOS5_" << (VB << "std" << "allocator_traits" << "allocator_traits<std::allocator<int>>" << "construct" << "construct<int, int const&>(std::allocator<int>&, int*, int const&)");
    }

    void testDemangler()
    {
        QFETCH(QString, mangled);
        QFETCH(QVector<QByteArray>, expectedDemangled);

        Demangler d;
        auto actualDemangled = d.demangle(mangled.toLatin1());
        if (actualDemangled != expectedDemangled) {
            qDebug() << actualDemangled;
            qDebug() << expectedDemangled;
        }
        QCOMPARE(actualDemangled, expectedDemangled);
    }
};

QTEST_MAIN(DemanglerTest)

#include "demangler_test.moc"