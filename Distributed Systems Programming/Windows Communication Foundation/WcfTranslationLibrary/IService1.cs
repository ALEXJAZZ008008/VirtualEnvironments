using System.ServiceModel;

namespace WcfTranslationLibrary
{
    [ServiceContract]

    public interface ITranslationService
    {
        [OperationContract]

        string Translate(string value);
    }
}
