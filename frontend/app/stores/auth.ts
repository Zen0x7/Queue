import {defineStore, acceptHMRUpdate} from "pinia";
import { Api } from "~/utils/api";
import {useNotifications} from "~/stores/notifications";

export const useAuth = defineStore("auth", () => {
    const notifications = useNotifications();

    const bearer = ref({
        token: '',
    });


    const form = ref({
        email: "admin@zendev.cl",
        password: "password",
    });

    const attempt = async () =>  {
        try {
            const response = await Api.call("post", "/api/auth/attempt", JSON.stringify(
                form.value
            ))
            bearer.value.token = response.data.token;
        } catch (e) {
            if (e.statusCode) {
                switch (e.statusCode) {
                    case 401:
                        await notifications.error({
                            title: "Error",
                            body: "La solicitud no ha sido authorizada.",
                            timeout: 5000,
                        });
                        break;
                    case 422:
                        await notifications.error({
                            title: "Error",
                            body: "La reglas de validación no han sido superadas.",
                            timeout: 5000,
                        });
                        break;
                }
            }
        }
    }

    return {
        bearer,
        form,
        attempt,
    };
})

if (import.meta.hot) {
    import.meta.hot.accept(acceptHMRUpdate(useAuth, import.meta.hot))
}